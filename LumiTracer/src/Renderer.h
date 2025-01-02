#pragma once

#include "Walnut/Image.h"
#include "glm/glm.hpp"

#include <memory>

#include "Ray.h"

class Camera;
struct Scene;

class Renderer {
public:
    Renderer();

    void Render(const Scene& scene, const Camera& camera);

    [[nodiscard]] std::shared_ptr<Walnut::Image> GetFinalImage() { return mFinalImage; }

    void ResetAccumulationFrames() { mAccumulationFrames = 1; }
    [[nodiscard]] glm::u32 GetAccumulationFrames() const { return mAccumulationFrames; }

private:
    struct HitPayload {
        glm::f32 hitDistance;
        glm::vec3 worldPosition;
        glm::vec3 worldNormal;
        glm::u32 objectIndex;
    };

    glm::vec4 PerPixel(const std::uint_fast32_t x, const std::uint_fast32_t y);

    HitPayload TraceRay(const Ray& ray);

    HitPayload ClosestHit(const Ray& ray, const glm::f32 hitDistance, const glm::u32 objectIndex);
    HitPayload Miss();

private:
    const Scene* mActiveScene;
    const Camera* mActiveCamera;
    std::shared_ptr<Walnut::Image> mFinalImage;
    glm::u32* mFinalImageData;
    glm::vec4* mAccumulationData;
    glm::u32 mAccumulationFrames;
};
