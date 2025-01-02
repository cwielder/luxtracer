#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

#include "Walnut/Random.h"

#include <iostream>

namespace {
	static glm::u32 ConvertToRGBA(const glm::vec4& color) {
		glm::u8 r = static_cast<glm::u8>(color.r * 255.0f);
		glm::u8 g = static_cast<glm::u8>(color.g * 255.0f);
		glm::u8 b = static_cast<glm::u8>(color.b * 255.0f);
		glm::u8 a = static_cast<glm::u8>(color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}

Renderer::Renderer()
    : mFinalImage()
    , mFinalImageData(nullptr)
{
    
}

void Renderer::Render(const Scene& scene, const Camera& camera) {
	const glm::u32vec2& viewport = camera.GetViewport();

	if (viewport.x == 0 || viewport.y == 0) {
		return;
	}

	if (!mFinalImage) {
		mFinalImage = std::make_shared<Walnut::Image>(viewport.x, viewport.y, Walnut::ImageFormat::RGBA);
	}
	
	if (mFinalImage->GetWidth() != viewport.x || mFinalImage->GetWidth() != viewport.y) {
		mFinalImage->Resize(viewport.x, viewport.y);
		delete[] mFinalImageData;
		mFinalImageData = new glm::u32[viewport.x * viewport.y];
	}

	std::cout << "Shading " << viewport.x * viewport.y << " pixels" << std::endl;

	Ray ray{ };
	ray.origin = camera.GetPosition();

	for (std::uint_fast32_t y = 0; y < viewport.y; y++) {
		for (std::uint_fast32_t x = 0; x < viewport.x; x++) {
			ray.direction = camera.GetRayDirections()[x + y * viewport.x];

			glm::vec4 color = this->TraceRay(scene, ray);
			color = glm::clamp(color, { 0.0f }, { 1.0f });
			mFinalImageData[x + y * viewport.x] = ConvertToRGBA(color);
		}
	}

	mFinalImage->SetData(mFinalImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray) {
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance

	constexpr glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	if (scene.spheres.size() == 0) [[unlikely]] {
		return clearColor;
	}

	const Sphere* closestSphere = nullptr;
	glm::f32 hitDistance = std::numeric_limits<glm::f32>::max();
	for (const auto& sphere : scene.spheres) {
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

		if (t1 < hitDistance) {
			closestSphere = &sphere;
			hitDistance = t1;
		}
	}

	if (closestSphere == nullptr) {
		return clearColor;
	}

	glm::vec3 origin = ray.origin - closestSphere->position;

	glm::vec3 hitPoint = origin + ray.direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	
	glm::f32 d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	return glm::vec4(closestSphere->material.albedo * d, 1.0f);
}
