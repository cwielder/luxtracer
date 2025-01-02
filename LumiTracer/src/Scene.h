#pragma once

#include "glm/glm.hpp"

#include <vector>

struct Material {
    glm::vec3 albedo;
    glm::f32 roughness;
    glm::f32 metallic;
};

struct Sphere {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::f32 radius = 1.0f;
    int materialIndex;
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Material> materials;
};
