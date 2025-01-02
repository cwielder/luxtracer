#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

Camera::Camera(glm::f32 verticalFOV, glm::f32 nearClip, glm::f32 farClip)
	: mVerticalFOV(verticalFOV)
	, mNearClip(nearClip)
	, mFarClip(farClip)
{
	mForwardDirection = glm::vec3(0, 0, -1);
	mPosition = glm::vec3(0, 0, 3);
}

bool Camera::OnUpdate(glm::f32 ts) {
	glm::vec2 mousePos = Walnut::Input::GetMousePosition();
	glm::vec2 delta = (mousePos - mLastMousePosition) * mSensitivity;
	mLastMousePosition = mousePos;

	if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right)) {
		Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
		return false;
	}

	Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(mForwardDirection, upDirection);

	glm::f32 speed = 5.0f;

	// Movement
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::W)) {
		mPosition += mForwardDirection * speed * ts;
		moved = true;
	} else if (Walnut::Input::IsKeyDown(Walnut::KeyCode::S)) {
		mPosition -= mForwardDirection * speed * ts;
		moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::A)) {
		mPosition -= rightDirection * speed * ts;
		moved = true;
	} else if (Walnut::Input::IsKeyDown(Walnut::KeyCode::D)) {
		mPosition += rightDirection * speed * ts;
		moved = true;
	}
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::Q)) {
		mPosition -= upDirection * speed * ts;
		moved = true;
	} else if (Walnut::Input::IsKeyDown(Walnut::KeyCode::E)) {
		mPosition += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f) {
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection), glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		mForwardDirection = glm::rotate(q, mForwardDirection);

		moved = true;
	}

	if (moved) {
		this->RecalculateView();
		this->RecalculateRayDirections();
	}

	return moved;
}

void Camera::OnResize(glm::u32 width, glm::u32 height) {
	if (width == mViewportWidth && height == mViewportHeight)
		return;

	mViewportWidth = width;
	mViewportHeight = height;

	this->RecalculateProjection();
	this->RecalculateRayDirections();
}

float Camera::GetRotationSpeed() {
	return 0.3f;
}

void Camera::RecalculateProjection() {
	mProjection = glm::perspectiveFov(glm::radians(mVerticalFOV), static_cast<glm::f32>(mViewportWidth), static_cast<glm::f32>(mViewportHeight), mNearClip, mFarClip);
	mInverseProjection = glm::inverse(mProjection);
}

void Camera::RecalculateView() {
	mView = glm::lookAt(mPosition, mPosition + mForwardDirection, glm::vec3(0, 1, 0));
	mInverseView = glm::inverse(mView);
}

void Camera::RecalculateRayDirections() {
	mRayDirections.resize(mViewportWidth * mViewportHeight);

	for (uint32_t y = 0; y < mViewportHeight; y++) {
		for (uint32_t x = 0; x < mViewportWidth; x++) {
			glm::vec2 coord = {
				static_cast<glm::f32>(x) / static_cast<glm::f32>(mViewportWidth),
				static_cast<glm::f32>(y) / static_cast<glm::f32>(mViewportHeight)
			};

			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 target = mInverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(mInverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			mRayDirections[x + y * mViewportWidth] = rayDirection;
		}
	}
}
