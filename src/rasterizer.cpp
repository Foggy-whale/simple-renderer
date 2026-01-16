#include <algorithm>
#include <any>
#include "rasterizer.h"

/* ======== 静态辅助接口部分 ======== */
template <typename T>
static T interpolate(float alpha, float beta, float gamma, const T& a, const T& b, const T& c) {
    return a * alpha + b * beta + c * gamma;
}

template<typename T>
static T get_avg(int ind, int ssaa, const std::vector<T>& buffer_data) {
    int sample_factor = ssaa * ssaa;
    T sum{}; 
    for(int i = 0; i < sample_factor; i++) {
        sum = sum + buffer_data[ind * sample_factor + i];
    }
    T avg = sum / (float)sample_factor;
    return avg;
}

static float signed_triangle_area(vec4 v1, vec4 v2, vec4 v3) {
    return 0.5f * ((v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x));
}

static std::pair<vec2, vec2> find_bounding_box(vec4 v1, vec4 v2, vec4 v3) {
    vec2 min = vec2(std::min({v1.x, v2.x, v3.x}), std::min({v1.y, v2.y, v3.y}));
    vec2 max = vec2(std::max({v1.x, v2.x, v3.x}), std::max({v1.y, v2.y, v3.y}));
    return {min, max};
}

static void assembly_triangle(std::array<Vertex, 3>& verts, Triangle& t) {
    for(int i = 0; i < 3; i++) {
        vec4 v = verts[i].pos;

        // Perspective Division
        v.x /= v.w, v.y /= v.w, v.z /= v.w;
        
        // Viewport Transform
        v.x = (v.x + 1.f) * 0.5f * width;
        v.y = (v.y + 1.f) * 0.5f * height;
        v.z = (1.f - v.z) * 0.5f;

        // Batch Assembly Attributes
        t.set_vertex(i, v);
        t.set_color(i, verts[i].color);
        t.set_world_pos(i, verts[i].world_pos);
        t.set_normal(i, verts[i].normal);
        t.set_tex_coord(i, verts[i].uv);
        t.set_tangent(i, verts[i].tangent);
        t.set_bitangent(i, verts[i].bitangent);
    }
}
/* ======== 静态辅助接口部分 ======== */

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

void Rasterizer::draw_triangle(const Triangle& triangle, const vec2& tri_min, const vec2& tri_max, const Tile& tile) {
    vec4 v1 = triangle.v[0], v2 = triangle.v[1], v3 = triangle.v[2];
    
    // Back-Face Culling
    float total_area = signed_triangle_area(v1, v2, v3);
    if(total_area < 1e-5) return;
    
    // 性能小trick: 化除法为乘法
    float inv_total_area = 1.f / total_area; 
    float inv_w1 = 1.f / v1.w, inv_w2 = 1.f / v2.w, inv_w3 = 1.f / v3.w;
    
    // Scissor Test
    int min_x = std::max((int)tile.x_start, (int)std::floor(tri_min.x));
    int max_x = std::min((int)tile.x_start + TILE_SIZE - 1, (int)std::ceil(tri_max.x));
    int min_y = std::max((int)tile.y_start, (int)std::floor(tri_min.y));
    int max_y = std::min((int)tile.y_start + TILE_SIZE - 1, (int)std::ceil(tri_max.y));

    // 使用 clamp 保证边界安全
    min_x = std::clamp(min_x, 0, width - 1);
    max_x = std::clamp(max_x, 0, width - 1);
    min_y = std::clamp(min_y, 0, height - 1);
    max_y = std::clamp(max_y, 0, height - 1);

    for(int x = min_x; x <= max_x; x++) {
        for(int y = min_y; y <= max_y; y++) {
            for(int si = 0; si < ssaa; si++) {
                for(int sj = 0; sj < ssaa; sj++) {
                    float x_sample = x + si * 1.f / ssaa + 0.5f / ssaa;
                    float y_sample = y + sj * 1.f / ssaa + 0.5f / ssaa;
                    
                    float alpha = signed_triangle_area(vec4(x_sample, y_sample, 0, 1), v2, v3) * inv_total_area;
                    float beta = signed_triangle_area(vec4(x_sample, y_sample, 0, 1), v3, v1) * inv_total_area;
                    float gamma = 1.f - alpha - beta;
                    if (alpha < 0 || beta < 0 || gamma < 0) continue; // 判定采样点是否在三角形内部

                    // Perspective-Correct Interpolation
                    // 经过数学推导，深度的倒数符合线性插值关系：1 / w = 1 / w1 * alpha + 1 / w2 * beta + 1 / w3 * gamma
                    // 同样可以推导出，矫正后的系数分别为：alpha_pc = alpha * (w / w1), beta_pc = beta * (w / w2), gamma_pc = gamma * (w / w3)
                    float w = 1.f / (inv_w1 * alpha + inv_w2 * beta + inv_w3 * gamma);
                    float alpha_pc = alpha * w * inv_w1;
                    float beta_pc = beta * w * inv_w2;
                    float gamma_pc = gamma * w * inv_w3;
                    
                    float z = interpolate(alpha_pc, beta_pc, gamma_pc, v1.z, v2.z, v3.z);
                    int ind = (x + y * width) * ssaa * ssaa + sj * ssaa + si;
                    if(z <= get_depth(ind)) continue; // 深度测试
                    
                    // 顶点属性插值
                    Vertex interpolated = Vertex::lerp(alpha_pc, beta_pc, gamma_pc, triangle);
                    interpolated.pos = {x_sample, y_sample, z, 1.0f};
                    
                    // 调用片段着色器处理当前像素
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
    // 清理现有的 Tile 索引列表
    for(auto& tile : tiles) {
        tile.triangle_indices.clear(); 
    }

    // 缓存所有三角形的数据，避免重复计算
    struct TriangleCache {
        Triangle t;
        vec2 min_xy, max_xy;
    };
    std::vector<TriangleCache> mesh_triangles(mesh.facet_vrt.size());

    // 为并行计算预处理装箱
    for(int i = 0; i < mesh.facet_vrt.size(); i++) {
        std::array<Vertex, 3> verts;
        
        // 委托顶点着色器处理每个顶点，获取处理后的顶点数据（Clip空间）
        verts[0] = currentShader->vertex(mesh, i, 0);
        verts[1] = currentShader->vertex(mesh, i, 1);
        verts[2] = currentShader->vertex(mesh, i, 2);
        
        // 将处理好的顶点装配成三角形
        Triangle t;
        assembly_triangle(verts, t);

        // 计算三角形的包围盒
        auto [min, max] = find_bounding_box(t.v[0], t.v[1], t.v[2]);

        // 将三角形缓存到 mesh_triangles 中
        mesh_triangles[i].t = t;
        mesh_triangles[i].min_xy = min, mesh_triangles[i].max_xy = max;
        
        // 计算影响了哪些 Tile
        int t_min_x = std::clamp((int)std::floor(min.x / TILE_SIZE), 0, tiles_x - 1);
        int t_max_x = std::clamp((int)std::ceil(max.x / TILE_SIZE), 0, tiles_x - 1);
        int t_min_y = std::clamp((int)std::floor(min.y / TILE_SIZE), 0, tiles_y - 1);
        int t_max_y = std::clamp((int)std::ceil(max.y / TILE_SIZE), 0, tiles_y - 1);

        // Bin-Packing 策略
        for(int ty = t_min_y; ty <= t_max_y; ty++) {
            for(int tx = t_min_x; tx <= t_max_x; tx++) {
                tiles[ty * tiles_x + tx].triangle_indices.push_back(i);
            }
        }
    }

    // 按照 Tile 并行渲染
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i].triangle_indices.empty()) continue;
        
        for (int tri_idx : tiles[i].triangle_indices) {
            draw_triangle(mesh_triangles[tri_idx].t, mesh_triangles[tri_idx].min_xy, mesh_triangles[tri_idx].max_xy, tiles[i]);
        }
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

TGAImage Rasterizer::to_tga_image(Buffers buffer) {
    TGAImage img;
    switch(buffer) {
        case Buffers::Color: {
            img = TGAImage(width, height, TGAImage::RGBA);
            // #pragma omp parallel for schedule(static)
            for(int i = 0; i < width * height; i++) {
                vec4 avg = get_avg(i, ssaa, framebuffer).clamp(0.0f, 1.0f);
                img.set(i % width, i / width, {static_cast<uint8_t>(avg.z * 255), 
                                               static_cast<uint8_t>(avg.y * 255), 
                                               static_cast<uint8_t>(avg.x * 255), 
                                               static_cast<uint8_t>(avg.w * 255)});
            }
            break;
        }
        case Buffers::Depth: {
            img = TGAImage(width, height, TGAImage::GRAYSCALE);
            // #pragma omp parallel for schedule(static)
            for(int i = 0; i < width * height; i++) {
                float avg = std::clamp(get_avg(i, ssaa, zbuffer), 0.0f, 1.0f);
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

    #pragma omp parallel for schedule(static)
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
