#pragma once
#include "geometry.h"

const std::string config_path = "configs/scene.json";

constexpr int width  = 1600;
constexpr int height = 1600;
constexpr float zNear = 0.1f;
constexpr float zFar = 100.f;

constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
constexpr float FLOAT_MIN = std::numeric_limits<float>::min();

constexpr int sm_width  = 3200;
constexpr int sm_height = 3200;

const vec2 poisson_disk[16] = {
    {-0.94201624, -0.39906216}, {0.94558609, -0.76890725}, {-0.094184101, -0.92938870}, {0.34495938, 0.29387760},
    {-0.91588581, 0.45771432}, {-0.81544232, -0.87912464}, {-0.38277543, 0.27676845}, {0.97484398, 0.75648379},
    {0.44323325, -0.97511554}, {0.53742981, -0.47373420}, {-0.51339162, -0.92262272}, {0.18731950, -0.30622198},
    {-0.24524135, 0.54291931}, {-0.40059289, -0.11043903}, {0.41212037, -0.65475873}, {0.90345513, -0.05487551}
};