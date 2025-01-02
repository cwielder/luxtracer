#pragma once

#include "Walnut/Image.h"
#include "glm/glm.hpp"

#include <memory>

class Renderer {
public:
    Renderer();

    void Render(const glm::u32vec2 viewport);

    [[nodiscard]] std::shared_ptr<Walnut::Image> GetFinalImage() { return mFinalImage; }

private:
    glm::vec4 PixelColor(const glm::vec2 coord);

    std::shared_ptr<Walnut::Image> mFinalImage;
    glm::u32* mFinalImageData;
};
