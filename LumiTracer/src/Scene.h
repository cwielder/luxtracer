#pragma once

#include "glm/glm.hpp"

#include <vector>

struct Material {
    glm::vec3 albedo = { 1.0f, 0.0f, 0.0f };
};

struct Sphere {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::f32 radius = 1.0f;
    Material material{ };
};

struct Scene {
    std::vector<Sphere> spheres;
};
