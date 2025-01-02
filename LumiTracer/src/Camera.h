#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera {
public:
	Camera(glm::f32 verticalFOV, glm::f32 nearClip, glm::f32 farClip);

	void OnUpdate(glm::f32 ts);
	void OnResize(glm::u32 width, glm::u32 height);

	[[nodiscard]] const glm::mat4& GetProjection() const { return mProjection; }
	[[nodiscard]] const glm::mat4& GetInverseProjection() const { return mInverseProjection; }
	[[nodiscard]] const glm::mat4& GetView() const { return mView; }
	[[nodiscard]] const glm::mat4& GetInverseView() const { return mInverseView; }

	[[nodiscard]] const glm::vec3& GetPosition() const { return mPosition; }
	[[nodiscard]] const glm::vec3& GetDirection() const { return mForwardDirection; }

	[[nodiscard]] glm::u32vec2 GetViewport() const { return { mViewportWidth, mViewportHeight }; }

	[[nodiscard]] const std::vector<glm::vec3>& GetRayDirections() const { return mRayDirections; }

	[[nodiscard]] glm::f32 GetRotationSpeed();

	void SetSensitivity(const glm::f32 sensitivity) { mSensitivity = sensitivity; }

private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();

	glm::mat4 mProjection{ 1.0f };
	glm::mat4 mView{ 1.0f };
	glm::mat4 mInverseProjection{ 1.0f };
	glm::mat4 mInverseView{ 1.0f };

	glm::f32 mVerticalFOV = 45.0f;
	glm::f32 mNearClip = 0.1f;
	glm::f32 mFarClip = 100.0f;

	glm::vec3 mPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 mForwardDirection{0.0f, 0.0f, 0.0f};

	// Cached ray directions
	std::vector<glm::vec3> mRayDirections;

	glm::vec2 mLastMousePosition{ 0.0f, 0.0f };

	glm::u32 mViewportWidth = 0, mViewportHeight = 0;

	glm::f32 mSensitivity = 0.002f;
};
