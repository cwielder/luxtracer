#pragma once

#include "Walnut/Image.h"
#include "glm/glm.hpp"

#include <memory>

#include "Ray.h"

class Camera;

class Renderer {
public:
    Renderer();

    void Render(const glm::u32vec2 viewport, const Camera& camera);

    [[nodiscard]] std::shared_ptr<Walnut::Image> GetFinalImage() { return mFinalImage; }

private:
    glm::vec4 TraceRay(const Ray& ray);

    std::shared_ptr<Walnut::Image> mFinalImage;
    glm::u32* mFinalImageData;
};
