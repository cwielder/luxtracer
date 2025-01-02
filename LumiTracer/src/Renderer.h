#pragma once

#include "Walnut/Image.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

#include "Ray.h"
#include "CounterIterator.h"

class Camera;
struct Scene;

class Renderer {
public:
    Renderer();

    void Render(const Scene& scene, const Camera& camera);

    [[nodiscard]] Walnut::Image* GetFinalImage() { return mFinalImage.get(); }

    void ResetAccumulationFrames() { mAccumulationFrames = 1; }
    [[nodiscard]] glm::u32 GetAccumulationFrames() const { return mAccumulationFrames; }

private:
    struct HitPayload {
        glm::f32 hitDistance;
        glm::vec3 worldPosition;
        glm::vec3 worldNormal;
        glm::u32 objectIndex;
    };

    glm::vec4 PerPixel(const glm::u32 x, const glm::u32 y) const;

    HitPayload TraceRay(const Ray& ray) const;

    HitPayload ClosestHit(const Ray& ray, const glm::f32 hitDistance, const glm::u32 objectIndex) const;
    HitPayload Miss() const;

private:
    const Scene* mActiveScene;
    const Camera* mActiveCamera;
    std::unique_ptr<Walnut::Image> mFinalImage;
    glm::u32* mFinalImageData;
    glm::vec4* mAccumulationData;
    glm::u32 mAccumulationFrames;
    CounterIterator mHorizIterBegin, mHorizIterEnd;
    CounterIterator mVertIterBegin, mVertIterEnd;
};
