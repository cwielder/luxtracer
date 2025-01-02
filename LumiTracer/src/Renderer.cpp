#include "Renderer.h"

#include "Walnut/Random.h"

#include <iostream>

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

			mFinalImageData[x + y * viewport.x] = this->GetPixelColor(coord);
		}
	}

	mFinalImage->SetData(mFinalImageData);
}

glm::u32 Renderer::GetPixelColor(const glm::vec2 coord) {
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance

	glm::vec3 rayDirection = glm::normalize(glm::vec3(coord, -1.0f));
	glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, -2.0f);
	glm::f32 radius = 0.5f;

	glm::f32 a = glm::dot(rayDirection, rayDirection);
	glm::f32 b = 2.0f * glm::dot(rayOrigin, rayDirection);
	glm::f32 c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	glm::f32 discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f) {
		return 0x00000000;
	} else {
		return 0xFFFFFFFF;
	}
}
