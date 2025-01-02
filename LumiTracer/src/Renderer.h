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

private:
    glm::vec4 TraceRay(const Scene& scene, const Ray& ray);

    std::shared_ptr<Walnut::Image> mFinalImage;
    glm::u32* mFinalImageData;
};
