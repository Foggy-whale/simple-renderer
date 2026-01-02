#include <algorithm>
#include "rasterizer.h"

static float cross_product(vec2 V1, vec2 V2) {
    return V1.x * V2.y - V1.y * V2.x;
}

static std::pair<vec2, vec2> find_bounding_box(vec2 v1, vec2 v2, vec2 v3) {
    vec2 min = vec2(std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.y}));
    vec2 max = vec2(std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.y}));
    return {min, max};
}

static bool inside_triangle(vec2 p, vec2 v1, vec2 v2, vec2 v3) {
    vec2 p1 = p - v1, p2 = p - v2, p3 = p - v3;
    float s1 = cross_product(p1, p2); // cross product of p1 and p2
    float s2 = cross_product(p2, p3); // cross product of p2 and p3
    float s3 = cross_product(p3, p1); // cross product of p3 and p1
    return (s1 >= 0 && s2 >= 0 && s3 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0);
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

void Rasterizer::triangle(vec2 v1, vec2 v2, vec2 v3, TGAColor color) {
    float total_area = cross_product(v2 - v1, v3 - v1);
    if (total_area < 1) return;

    auto [min, max] = find_bounding_box(v1, v2, v3);
    #pragma omp parallel for
    for (int x = min.x; x <= max.x; x++) {
        for (int y = min.y; y <= max.y; y++) {
            if (inside_triangle(vec2(x, y), v1, v2, v3)) {
                framebuffer.set(x, y, color);
            }
        }
    }
}
