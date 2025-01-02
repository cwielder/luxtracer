#include "Renderer.h"

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

void Renderer::Render(const glm::u32vec2 viewport) {
	if (!mFinalImage) {
		mFinalImage = std::make_shared<Walnut::Image>(viewport.x, viewport.y, Walnut::ImageFormat::RGBA);
	}
	
	if (mFinalImage->GetWidth() != viewport.x || mFinalImage->GetWidth() != viewport.y) {
		mFinalImage->Resize(viewport.x, viewport.y);
		delete[] mFinalImageData;
		mFinalImageData = new glm::u32[viewport.x * viewport.y];
	}

	std::cout << "Shading " << viewport.x * viewport.y << " pixels" << std::endl;

	for (std::uint_fast32_t y = 0; y < viewport.y; y++) {
		for (std::uint_fast32_t x = 0; x < viewport.x; x++) {
			glm::vec2 coord = {
				static_cast<glm::f32>(x) / static_cast<glm::f32>(viewport.x),
				static_cast<glm::f32>(y) / static_cast<glm::f32>(viewport.y)
			};

			coord = coord * 2.0f - 1.0f; // Map to [-1, 1]

			glm::vec4 color = this->PixelColor(coord);
			color = glm::clamp(color, { 0.0f }, { 1.0f });
			mFinalImageData[x + y * viewport.x] = ConvertToRGBA(color);
		}
	}

	mFinalImage->SetData(mFinalImageData);
}

glm::vec4 Renderer::PixelColor(const glm::vec2 coord) {
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance

	glm::vec3 rayDirection = glm::vec3(coord, -1.0f); //rayDirection = glm::normalize(rayDirection);
	glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::f32 sphereRadius = 0.5f;
	glm::vec3 sphereColor = glm::vec3(1, 0, 1);

	glm::f32 a = glm::dot(rayDirection, rayDirection);
	glm::f32 b = 2.0f * glm::dot(rayOrigin, rayDirection);
	glm::f32 c = glm::dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;

	glm::f32 discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f) {
		return glm::vec4(0, 0, 0, 1);
	}

	// (-b +- sqrt(discriminant)) / 2a
	glm::f32 t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	glm::f32 t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

	glm::vec3 hitPoint = rayOrigin + rayDirection * t1;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	
	glm::f32 d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	return glm::vec4(sphereColor * d, 1.0f);
}
