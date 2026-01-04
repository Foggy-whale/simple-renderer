#include <algorithm>
#include "random.h"
#include "rasterizer.h"

template <typename T>
static T interpolate(float alpha, float beta, float gamma, const T& a, const T& b, const T& c) {
    return a * alpha + b * beta + c * gamma;
}

static vec4 to_vec4(const vec3& v) {
    return vec4(v.x, v.y, v.z, 1);
}

static float signed_triangle_area(vec4 v1, vec4 v2, vec4 v3) {
    return 0.5f * ((v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x));
}

static std::pair<vec2, vec2> find_bounding_box(vec4 v1, vec4 v2, vec4 v3) {
    vec2 min = vec2(std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.y}));
    vec2 max = vec2(std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.y}));
    return {min, max};
}

void Rasterizer::draw_line(vec2 v1, vec2 v2, TGAColor color) {
    float x1_s = v1.x * ssaa, y1_s = v1.y * ssaa;
    float x2_s = v2.x * ssaa, y2_s = v2.y * ssaa;

    int x = (int)x1_s, y = (int)y1_s;
    int target_x = (int)x2_s, target_y = (int)y2_s;

    int dx = abs(target_x - x), sx = x < target_x ? 1 : -1;
    int dy = -abs(target_y - y), sy = y < target_y ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        int px = x / ssaa;
        int py = y / ssaa;
        int si = x % ssaa;
        int sj = y % ssaa;

        if (px >= 0 && px < width && py >= 0 && py < height) {
            int ind = (px + py * width) * ssaa * ssaa + sj * ssaa + si;
            set_pixel(ind, vec3(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f));
        }

        if (x == target_x && y == target_y) break;

        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void Rasterizer::draw_triangle(const Triangle& triangle) {
    vec4 v1 = triangle.v[0], v2 = triangle.v[1], v3 = triangle.v[2];
    float total_area = signed_triangle_area(v1, v2, v3);
    // Relaxed check to allow negative area (different winding order due to Y-flip)
    // Only cull degenerate triangles with near-zero area
    if (std::abs(total_area) < 1e-5) return;

    vec3 c1 = triangle.color[0], c2 = triangle.color[1], c3 = triangle.color[2];
    float inv_total_area = 1.f / total_area;
    auto [min, max] = find_bounding_box(v1, v2, v3);
    
    // Bounds checking to prevent segfault
    int min_x = std::max(0, (int)min.x);
    int max_x = std::min(width - 1, (int)max.x);
    int min_y = std::max(0, (int)min.y);
    int max_y = std::min(height - 1, (int)max.y);

    #pragma omp parallel for
    for (int x = min_x; x <= max_x; x++) {
        for (int y = min_y; y <= max_y; y++) {
            for(int si = 0; si < ssaa; si++) {
                for(int sj = 0; sj < ssaa; sj++) {
                    float x_sample = x + si * 1.f / ssaa + 0.5f / ssaa;
                    float y_sample = y + sj * 1.f / ssaa + 0.5f / ssaa;
                    
                    float alpha = signed_triangle_area(vec4(x_sample, y_sample, 0, 1), v2, v3) * inv_total_area;
                    float beta = signed_triangle_area(vec4(x_sample, y_sample, 0, 1), v3, v1) * inv_total_area;
                    float gamma = 1.f - alpha - beta;
                    if (alpha < 0 || beta < 0 || gamma < 0) continue;
                    // Perspective-Correct Interpolation
                    float w_reciprocal = 1.f / (alpha / v1.w + beta / v2.w + gamma / v3.w);
                    float alpha_pc = alpha * (w_reciprocal / v1.w);
                    float beta_pc = beta * (w_reciprocal / v2.w);
                    float gamma_pc = gamma * (w_reciprocal / v3.w);
                    float z = interpolate(alpha_pc, beta_pc, gamma_pc, v1.z, v2.z, v3.z);
                    int ind = (x + y * width) * ssaa * ssaa + sj * ssaa + si;
                    if(z <= get_depth(ind)) continue;
                    vec3 color = interpolate(alpha_pc, beta_pc, gamma_pc, c1, c2, c3);
                    set_depth(ind, z);
                    set_pixel(ind, color);   
                }
            }
        }
    }
}

void Rasterizer::draw_model(const Model& m, const mat4& mvp) {
    for(int i = 0; i < m.nmeshes(); i++) {
        const auto& mesh = m.mesh(i);
        for (int j = 0; j < mesh.facet_vrt.size(); j++) {
            Triangle t;
            vec4 v0 = to_vec4(mesh.verts[mesh.facet_vrt[j][0]]);
            vec4 v1 = to_vec4(mesh.verts[mesh.facet_vrt[j][1]]);
            vec4 v2 = to_vec4(mesh.verts[mesh.facet_vrt[j][2]]);
            
            // MVP Transformation
            v0 = mvp * v0;
            v1 = mvp * v1;
            v2 = mvp * v2;
            // Perspective-Division
            auto perspective_divide = [](vec4& v) {
                v.x /= v.w;
                v.y /= v.w;
                v.z /= v.w;
            };
            perspective_divide(v0);
            perspective_divide(v1);
            perspective_divide(v2);
            // Viewport Transformation
            auto viewport_transform = [&](vec4& v) {
                v.x = (v.x + 1.f) * 0.5f * width;
                v.y = (v.y + 1.f) * 0.5f * height;
                v.z = (1.f - v.z) * 0.5f; // Reversed-Z
            };
            viewport_transform(v0);
            viewport_transform(v1);
            viewport_transform(v2);

            t.setVertex(0, v0);
            t.setVertex(1, v1);
            t.setVertex(2, v2);

            Random rng;
            float r = rng(256), g = rng(256), b = rng(256); 
            t.setColor(0, r, g, b);
            t.setColor(1, r, g, b);
            t.setColor(2, r, g, b);

            draw_triangle(t);
        }
    }
}

void Rasterizer::draw(const Scene& scene) {
    Camera camera = scene.getCamera();
    view = camera.get_view_matrix();
    projection = camera.get_projection_matrix();
    mat4 vp = projection * view;
    for(auto m : scene.getModels()) {
        draw_model(m, vp * m.get_model_matrix());
    }
}

void Rasterizer::save_zbuffer_as(const std::string& filename) {
    int sample_factor = ssaa * ssaa;
    TGAImage img(width, height, TGAImage::GRAYSCALE);
    
    float min_depth = 1.0f;
    float max_depth = 0.0f;

    for(int i = 0; i < width * height * sample_factor; i++) {
        if(zbuffer[i] <= 1e-6f) continue; 
        if(zbuffer[i] < min_depth) min_depth = zbuffer[i];
        if(zbuffer[i] > max_depth) max_depth = zbuffer[i];
    }

    for(int i = 0; i < width * height; i++) {
        float sum_depth = 0;
        int count = 0;
        for(int j = 0; j < sample_factor; j++) {
            float z = zbuffer[i * sample_factor + j];
            if (z > 1e-6f) {
                sum_depth += z;
                count++;
            }
        }
        if (!count) {
            img.set(i % width, i / width, {0}); // 全背景像素设为黑
            continue;
        }
        float depth = sum_depth / count;
        depth = std::clamp((depth - min_depth) / (max_depth - min_depth), 0.0f, 1.0f);
        depth = std::pow(depth, 0.5f); 
        img.set(i % width, i / width, {static_cast<uint8_t>(depth * 255.0f)});
    }
    img.write_tga_file(filename);
}