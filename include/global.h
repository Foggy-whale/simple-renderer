#pragma once
#include "geometry.h"

const std::string config_path = "configs/scene.json";

constexpr int width  = 1600;
constexpr int height = 1600;

struct Light {
    vec3 position;
    vec3 intensity;
};
