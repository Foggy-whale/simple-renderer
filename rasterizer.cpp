#include <algorithm>
#include "random.h"
#include "rasterizer.h"

static float signed_triangle_area(vec3 v1, vec3 v2, vec3 v3) {
    return .5 * ((v2.y - v1.y) * (v2.x + v1.x) + (v3.y - v2.y) * (v3.x + v2.x) + (v1.y - v3.y) * (v1.x + v3.x));
}

static std::pair<vec2, vec2> find_bounding_box(vec3 v1, vec3 v2, vec3 v3) {
    vec2 min = vec2(std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.y}));
    vec2 max = vec2(std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.y}));
    return {min, max};
}

void Rasterizer::line(vec2 v1, vec2 v2, TGAColor color) {
    int dx = abs(v2.x - v1.x), sx = v1.x < v2.x ? 1 : -1;
    int dy = -abs(v2.y - v1.y), sy = v1.y < v2.y ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        framebuffer.set(v1.x, v1.y, color);
        if (v1.x == v2.x && v1.y == v2.y) break;
        
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            v1.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            v1.y += sy;
        }
    }
}

void Rasterizer::triangle(vec3 v1, vec3 v2, vec3 v3) {
    float total_area = signed_triangle_area(v1, v2, v3);
    if (total_area < 1) return;

    TGAColor c1, c2, c3;
    Random rng;
    c1 = {static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), 255};
    c2 = {static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), 255};
    c3 = {static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), static_cast<uint8_t>(rng(256)), 255};

    auto [min, max] = find_bounding_box(v1, v2, v3);
    #pragma omp parallel for
    for (int x = min.x; x <= max.x; x++) {
        for (int y = min.y; y <= max.y; y++) {
            float alpha = signed_triangle_area(vec3(x, y, 0), v2, v3) / total_area;
            float beta = signed_triangle_area(vec3(x, y, 0), v3, v1) / total_area;
            float gamma = signed_triangle_area(vec3(x, y, 0), v1, v2) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            TGAColor c = {static_cast<uint8_t>(alpha * c1[0] + beta * c2[0] + gamma * c3[0]),
                          static_cast<uint8_t>(alpha * c1[1] + beta * c2[1] + gamma * c3[1]),
                          static_cast<uint8_t>(alpha * c1[2] + beta * c2[2] + gamma * c3[2]),
                          255};
            framebuffer.set(x, y, c);
        }
    }
}

