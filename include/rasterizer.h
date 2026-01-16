#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include "triangle.h"
#include "model.h"
#include "scene.h"
#include "shader.h"

enum class Buffers {
    Color = 1 << 0,
    Depth = 1 << 1
};
inline Buffers operator|(Buffers a, Buffers b) {
    return Buffers((int)a | (int)b);
}
inline Buffers operator&(Buffers a, Buffers b) {
    return Buffers((int)a & (int)b);
}

/* Tile-Based 并行优化策略 */
const int TILE_SIZE = 32;
const int tiles_x = (width + TILE_SIZE - 1) / TILE_SIZE;
const int tiles_y = (height + TILE_SIZE - 1) / TILE_SIZE;

struct Tile {
    int x_start, y_start;
    std::vector<int> triangle_indices; // 该 Tile 覆盖的三角形索引
};

class Rasterizer {
private:
    std::vector<Tile> tiles; // 所有 Tile 信息

    /* 渲染参数 */
    int width, height;
    int ssaa = 1;
    std::vector<vec4> framebuffer;
    std::vector<float> zbuffer;

    ShaderContext context; // 渲染上下文
    IShader* currentShader; // 当前Shader类型
    
    /* 资源管理池 */
    ModelManager* modelMgr = nullptr;
    ShaderManager* shaderMgr = nullptr;
    TextureManager* texMgr = nullptr;
    MaterialManager* matMgr = nullptr;
public:
    Rasterizer() = default;
    Rasterizer(int width, int height) : width(width), height(height) {
        framebuffer.resize(width * height, vec4(0, 0, 0, 1.f));
        zbuffer.resize(width * height);

        tiles.resize(tiles_x * tiles_y);
        for(int i = 0; i < tiles.size(); i++) {
            tiles[i].x_start = (i % tiles_x) * TILE_SIZE;
            tiles[i].y_start = (i / tiles_x) * TILE_SIZE;
        }
    }
    ~Rasterizer() = default;

    void draw(const Scene& scene);
    
    TGAImage to_tga_image(Buffers buffer);
    void save_as(const std::string& filename);
    void save_zbuffer_as(const std::string& filename);
    void clear(Buffers buffer) {
        if((buffer & Buffers::Color) == Buffers::Color) std::fill(framebuffer.begin(), framebuffer.end(), vec4(0, 0, 0, 1.f));
        if((buffer & Buffers::Depth) == Buffers::Depth) std::fill(zbuffer.begin(), zbuffer.end(), 0);
    }
    
    void set_depth(const int& ind, const float& z) { zbuffer[ind] = z; }
    void set_pixel(const int& ind, const vec4& rgba) {
        /* Alpha Blending */ 
        float a = rgba.w; // source alpha
        // float a = 1;
    
        // 注意只对RGB通道进行混合
        framebuffer[ind].x = rgba.x * a + framebuffer[ind].x * (1.0f - a);
        framebuffer[ind].y = rgba.y * a + framebuffer[ind].y * (1.0f - a);
        framebuffer[ind].z = rgba.z * a + framebuffer[ind].z * (1.0f - a);
        
        // 目标Alpha设为1.0f，确保后续渲染不会被当前片段遮挡
        framebuffer[ind].w = 1.0f;
        // framebuffer[ind].w = a + framebuffer[ind].w * (1.0f - a);
    }

    void bind_managers(std::unique_ptr<ModelManager>& modelMgr, std::unique_ptr<ShaderManager>& shaderMgr, std::unique_ptr<TextureManager>& texMgr, std::unique_ptr<MaterialManager>& matMgr) {
        this->modelMgr = modelMgr.get();
        this->shaderMgr = shaderMgr.get();
        this->texMgr = texMgr.get();
        this->matMgr = matMgr.get();
    }

    float get_depth(const int& ind) const { return zbuffer[ind]; }
    vec4 get_pixel(const int& ind) const { return framebuffer[ind]; }
    std::vector<vec4>& get_framebuffer() { return framebuffer; }
    std::vector<float>& get_zbuffer() { return zbuffer; }
    
    void enable_ssaa(const int& ssaa) { 
        this->ssaa = ssaa;
        framebuffer.resize(width * height * ssaa * ssaa, vec4(0, 0, 0, 1.f));
        zbuffer.resize(width * height * ssaa * ssaa);
    }
private:
    /* 把绘制过程划分成更具体的层次，
     * 1. 绘制线
     * 2. 绘制三角形
     * 3. 绘制网格
     * 4. 绘制实体
     */
    void draw_line(vec2 v1, vec2 v2, TGAColor color);
    void draw_triangle(const Triangle& triangle, const vec2& tri_min, const vec2& tri_max, const Tile& tile);
    void draw_mesh(const Mesh& mesh);
    void draw_entity(const Entity* e);
};