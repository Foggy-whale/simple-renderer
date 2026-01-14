#include <algorithm>
#include <any>
#include "rasterizer.h"

template <typename T>
static T interpolate(float alpha, float beta, float gamma, const T& a, const T& b, const T& c) {
    return a * alpha + b * beta + c * gamma;
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
            set_pixel(ind, vec4(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f, 1.f));
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
    if (std::abs(total_area) < 1e-5) return;

    // Construct vertices from Triangle data for interpolation
    Vertex vert1, vert2, vert3;
    vert1.pos = v1; vert1.color = triangle.color[0]; vert1.normal = triangle.normal[0]; vert1.uv = triangle.tex_coord[0]; vert1.world_pos = triangle.world_pos[0]; vert1.tangent = triangle.tangent[0]; vert1.bitangent = triangle.bitangent[0];
    vert2.pos = v2; vert2.color = triangle.color[1]; vert2.normal = triangle.normal[1]; vert2.uv = triangle.tex_coord[1]; vert2.world_pos = triangle.world_pos[1]; vert2.tangent = triangle.tangent[1]; vert2.bitangent = triangle.bitangent[1];
    vert3.pos = v3; vert3.color = triangle.color[2]; vert3.normal = triangle.normal[2]; vert3.uv = triangle.tex_coord[2]; vert3.world_pos = triangle.world_pos[2]; vert3.tangent = triangle.tangent[2]; vert3.bitangent = triangle.bitangent[2];

    float inv_total_area = 1.f / total_area;
    auto [min, max] = find_bounding_box(v1, v2, v3);
    
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
                    if(z <= get_depth(ind)) continue; // 深度测试
                    
                    /* 计算插值后的顶点属性 */
                    Vertex interpolated;
                    interpolated.pos = {x_sample, y_sample, z, 1.0f}; 
                    interpolated.color = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.color, vert2.color, vert3.color);
                    interpolated.normal = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.normal, vert2.normal, vert3.normal).normalized();
                    interpolated.uv = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.uv, vert2.uv, vert3.uv);
                    interpolated.world_pos = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.world_pos, vert2.world_pos, vert3.world_pos);
                    interpolated.tangent = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.tangent, vert2.tangent, vert3.tangent);
                    interpolated.bitangent = interpolate(alpha_pc, beta_pc, gamma_pc, vert1.bitangent, vert2.bitangent, vert3.bitangent);
                    
                    vec4 rgba;
                    bool discard = currentShader->fragment(interpolated, rgba);

                    if (!discard) {
                        set_depth(ind, z);
                        set_pixel(ind, rgba);   
                    }
                }
            }
        }
    }
}

void Rasterizer::draw_mesh(const Mesh& mesh) {
    for (int j = 0; j < mesh.facet_vrt.size(); j++) {
        Triangle t;

        // 委托顶点着色器处理每个顶点，获取处理后的顶点数据（Clip空间）
        Vertex v0_vert = currentShader->vertex(mesh, j, 0);
        Vertex v1_vert = currentShader->vertex(mesh, j, 1);
        Vertex v2_vert = currentShader->vertex(mesh, j, 2);

        vec4 v0 = v0_vert.pos;
        vec4 v1 = v1_vert.pos;
        vec4 v2 = v2_vert.pos;
        
        auto perspective_divide = [](vec4& v) {
            v.x /= v.w;
            v.y /= v.w;
            v.z /= v.w;
        };
        perspective_divide(v0);
        perspective_divide(v1);
        perspective_divide(v2);
        
        auto viewport_transform = [&](vec4& v) {
            v.x = (v.x + 1.f) * 0.5f * width;
            v.y = (v.y + 1.f) * 0.5f * height;
            v.z = (1.f - v.z) * 0.5f;
        };
        viewport_transform(v0);
        viewport_transform(v1);
        viewport_transform(v2);

        t.set_vertex(0, v0);
        t.set_vertex(1, v1);
        t.set_vertex(2, v2);
        
        t.set_color(0, v0_vert.color);
        t.set_color(1, v1_vert.color);
        t.set_color(2, v2_vert.color);
        
        t.set_world_pos(0, v0_vert.world_pos);
        t.set_world_pos(1, v1_vert.world_pos);
        t.set_world_pos(2, v2_vert.world_pos);

        t.set_normal(0, v0_vert.normal);
        t.set_normal(1, v1_vert.normal);
        t.set_normal(2, v2_vert.normal);
        
        t.set_tex_coord(0, v0_vert.uv);
        t.set_tex_coord(1, v1_vert.uv);
        t.set_tex_coord(2, v2_vert.uv);
        
        t.set_tangent(0, v0_vert.tangent);
        t.set_tangent(1, v1_vert.tangent);
        t.set_tangent(2, v2_vert.tangent);
        
        t.set_bitangent(0, v0_vert.bitangent);
        t.set_bitangent(1, v1_vert.bitangent);
        t.set_bitangent(2, v2_vert.bitangent);  
        
        draw_triangle(t);
    }
}


void Rasterizer::draw_entity(const Entity* e) {
    Model* m = modelMgr->get_model(e->get_model_id());
    mat4 model = e->get_matrix();

    // Set shader context that are common
    // Helper lambda to update shader based on type
    auto update_shader = [&](const Mesh& mesh) {
        Material* mtl = matMgr->get_material(mesh.material_id);
        
        /* 设置模型参数 */
        context.mtl = mtl;
        context.model = model;
        context.mvp = context.vp * context.model;

        currentShader = shaderMgr->get_shader(mtl->shader_id);
        currentShader->bind_context(&context);
    };

    for(int i = 0; i < m->nmeshes(); i++) {
        const Mesh& mesh = m->mesh(i);
        update_shader(mesh);
        draw_mesh(mesh);
    }
}

void Rasterizer::draw(const Scene& scene) {
    /* 导入相机参数 */
    const Camera& camera = scene.get_camera();
    context.eye_pos = camera.get_eye();
    context.vp = camera.get_projection_matrix() * camera.get_view_matrix();

    /* 导入场景中的所有光源（通过指针引用，避免拷贝）*/
    context.lights = &scene.get_lights();

    /* 导入纹理管理器 */
    context.texMgr = texMgr;

    for(auto e : scene.get_entities()) draw_entity(e);
}

template<typename T>
inline T Rasterizer::get_avg(const int& ind, const std::vector<T>& buffer_data) {
    int sample_factor = ssaa * ssaa;
    T sum{}; 
    for(int i = 0; i < sample_factor; i++) {
        sum = sum + buffer_data[ind * sample_factor + i];
    }
    T avg = sum / (float)sample_factor;
    return avg;
}

TGAImage Rasterizer::to_tga_image(Buffers buffer) {
    TGAImage img;
    switch(buffer) {
        case Buffers::Color: {
            img = TGAImage(width, height, TGAImage::RGBA);
            for(int i = 0; i < width * height; i++) {
                vec4 avg = get_avg(i, framebuffer).clamp(0.0f, 1.0f);
                img.set(i % width, i / width, {static_cast<uint8_t>(avg.z * 255), 
                                               static_cast<uint8_t>(avg.y * 255), 
                                               static_cast<uint8_t>(avg.x * 255), 
                                               static_cast<uint8_t>(avg.w * 255)});
            }
            break;
        }
        case Buffers::Depth: {
            img = TGAImage(width, height, TGAImage::GRAYSCALE);
            for(int i = 0; i < width * height; i++) {
                float avg = std::clamp(get_avg(i, zbuffer), 0.0f, 1.0f);
                img.set(i % width, i / width, {static_cast<uint8_t>(avg * 255)});
            }
            break;
        }
    }
    return img;
}


void Rasterizer::save_as(const std::string &filename) {
    TGAImage img = to_tga_image(Buffers::Color);
    img.write_tga_file(filename);
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