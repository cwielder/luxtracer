#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

#include "Walnut/Random.h"

#include <execution>

namespace {
	static glm::u32 convertToRGBA(const glm::vec4& color) {
		glm::u8 r = static_cast<glm::u8>(color.r * 255.0f);
		glm::u8 g = static_cast<glm::u8>(color.g * 255.0f);
		glm::u8 b = static_cast<glm::u8>(color.b * 255.0f);
		glm::u8 a = static_cast<glm::u8>(color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	static glm::u32 hashPCG(const glm::u32 input) {
		const glm::u32 state = input * 747796405U + 2891336453U, word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
		return (word >> 22U) ^ word;
	}

	static glm::f32 randF32(glm::u32& seed) {
		seed = hashPCG(seed);
		return static_cast<glm::f32>(seed) / static_cast<glm::f32>(std::numeric_limits<glm::u32>::max());
	}

	static glm::vec3 randV3(glm::u32& seed, const glm::f32 min, const glm::f32 max) {
		return glm::vec3(randF32(seed) * (max - min) + min, randF32(seed) * (max - min) + min, randF32(seed) * (max - min) + min);
	}

	static glm::vec3 randV3Unit(glm::u32& seed) {
		return glm::normalize(randV3(seed, -1.0f, 1.0f));
	}

	static glm::f32 lerp(const glm::f32 a, const glm::f32 b, const glm::f32 t) {
		return (1.0f - t) * a + t * b;
	}

	static glm::vec3 lerp(const glm::vec3 a, const glm::vec3 b, const glm::f32 t) {
		return {
			lerp(a.x, b.x, t),
			lerp(a.y, b.y, t),
			lerp(a.z, b.z, t)
		};
	}
}

Renderer::Renderer()
	: mActiveScene(nullptr)
	, mActiveCamera(nullptr)
	, mFinalImage()
	, mFinalImageData(nullptr)
	, mAccumulationData(nullptr)
	, mAccumulationFrames(1)
	, mMaxBounces(1)
	, mFlags(0)
{ }

void Renderer::Render(const Scene& scene, const Camera& camera) {
	mActiveScene = &scene;
	mActiveCamera = &camera;

	const glm::u32vec2& viewport = camera.GetViewport();

	if (viewport.x == 0 || viewport.y == 0) {
		return;
	}

	if (!mFinalImage) {
		mFinalImage = std::make_unique<Walnut::Image>(viewport.x, viewport.y, Walnut::ImageFormat::RGBA);
	}
	
	if (!mFinalImageData || !mAccumulationData || mFinalImage->GetWidth() != viewport.x || mFinalImage->GetHeight() != viewport.y) {
		mFinalImage->Resize(viewport.x, viewport.y);
		delete[] mFinalImageData;
		mFinalImageData = new glm::u32[viewport.x * viewport.y];
		delete[] mAccumulationData;
		mAccumulationData = new glm::vec4[viewport.x * viewport.y];

		mHorizIterBegin = CounterIterator(0);
		mHorizIterEnd = CounterIterator(viewport.x);
		mVertIterBegin = CounterIterator(0);
		mVertIterEnd = CounterIterator(viewport.y);

		this->ResetAccumulationFrames();
	}

	if (mAccumulationFrames == 1) {
		std::memset(mAccumulationData, 0, viewport.x * viewport.y * sizeof(glm::vec4));
	}

	std::for_each(std::execution::par, mVertIterBegin, mVertIterEnd, [this, viewport](const glm::u32 y) {
		std::for_each(mHorizIterBegin, mHorizIterEnd, [this, y, viewport](const glm::u32 x) {
			glm::vec4 color = this->PerPixel(x, y);

			const glm::u32 pixelIndex = x + y * viewport.x;

			mAccumulationData[pixelIndex] += color;

			glm::vec4 accumulation = mAccumulationData[pixelIndex];

			color = glm::clamp(mAccumulationData[pixelIndex] / static_cast<glm::f32>(mAccumulationFrames), { 0.0f }, { 1.0f });
			mFinalImageData[pixelIndex] = convertToRGBA(color);
		});
	});

	mFinalImage->SetData(mFinalImageData);

	if (mFlags & Flags::Accumulate)
		mAccumulationFrames++;

	mActiveScene = nullptr;
	mActiveCamera = nullptr;
}

glm::vec4 Renderer::PerPixel(const glm::u32 x, const glm::u32 y) const {
	Ray ray = {
		.origin = mActiveCamera->GetPosition(),
		.direction = mActiveCamera->GetRayDirections()[x + y * mActiveCamera->GetViewport().x]
	};
	
	glm::u32 seed = (x + y * mActiveCamera->GetViewport().x) * mAccumulationFrames;

	glm::vec3 light = glm::vec3(0.0f);
	glm::vec3 contribution = glm::vec3(1.0f);

	for (int i = 0; i < mMaxBounces; i++) {
		seed += i;

		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.hitDistance > 0.0f) {
			const Sphere& sphere = mActiveScene->spheres[payload.objectIndex];
			const Material& material = mActiveScene->materials[sphere.materialIndex];

			const glm::vec3 randomAngle = randV3Unit(seed);
			
			const glm::vec3 diffuseDir = glm::normalize(payload.worldNormal + randomAngle);
			const glm::vec3 specularDir = glm::reflect(ray.direction, payload.worldNormal);

			const bool specular = material.metallic >= randF32(seed);

			ray.origin = payload.worldPosition + payload.worldNormal * 0.0001f;
			ray.direction = lerp(specularDir, diffuseDir, material.roughness * !specular);

			light += material.emissiveColor * material.emissiveStrength * contribution;
			contribution *= lerp(material.albedo, glm::vec3(1.0f), specular);
		} else {
			break;
		}
	}

	return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const {
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance

	constexpr glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	if (mActiveScene->spheres.size() == 0) [[unlikely]] {
		return this->Miss();
	}

	glm::u32 objectIndex = std::numeric_limits<glm::u32>::max();
	glm::f32 hitDistance = std::numeric_limits<glm::f32>::max();
	for (int i = 0; i < mActiveScene->spheres.size(); i++) {
		const auto& sphere = mActiveScene->spheres[i];

		glm::vec3 origin = ray.origin - sphere.position;

		glm::f32 a = glm::dot(ray.direction, ray.direction);
		glm::f32 b = 2.0f * glm::dot(origin, ray.direction);
		glm::f32 c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

		glm::f32 discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f) {
			continue;
		}

		// (-b +- sqrt(discriminant)) / 2a

		glm::f32 t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (t1 > 0.0f && t1 < hitDistance) {
			objectIndex = i;
			hitDistance = t1;
		}
	}

	if (objectIndex == std::numeric_limits<glm::u32>::max()) {
		return this->Miss();
	} else {
		return this->ClosestHit(ray, hitDistance, objectIndex);
	}
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, const glm::f32 hitDistance, const glm::u32 objectIndex) const {
	const auto& sphere = mActiveScene->spheres[objectIndex];

	glm::vec3 origin = ray.origin - sphere.position;

	glm::vec3 worldPosition = origin + ray.direction * hitDistance;
	glm::vec3 normal = glm::normalize(worldPosition);

	return {
		.hitDistance = hitDistance,
		.worldPosition = sphere.position + worldPosition,
		.worldNormal = normal,
		.objectIndex = objectIndex,
	};
}

Renderer::HitPayload Renderer::Miss() const {
	return {
		.hitDistance = -1.0f
	};
}
