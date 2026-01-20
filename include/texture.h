#pragma once
#include <map>
#include "tgaimage.h"
#include "geometry.h"

namespace Tex {
    enum class Interpolation { NEAREST, BILINEAR };
    enum class WrapMode { REPEAT, CLAMP, MIRROR };
}
using Interpolation = Tex::Interpolation;
using WrapMode = Tex::WrapMode;

struct Texture {
    TGAImage* data;           // 原始图片数据
    Interpolation mode;       // NEAREST, BILINEAR
    WrapMode wrap;            // REPEAT, CLAMP, MIRROR

    Texture(const std::string filename, Interpolation m = Interpolation::BILINEAR, WrapMode w = WrapMode::REPEAT) 
        : data(new TGAImage(filename.c_str())), mode(m), wrap(w) {
            data->flip_vertically();
        }

    float handle_wrap(float v) const; 
    TGAColor sample_uv(vec2 uv) const;
};

class TextureManager {
private:
    int next_id = 0; // 将要分配的纹理id，从0开始递增
    std::unordered_map<std::string, int> texture_map; // path -> id
    std::vector<std::unique_ptr<Texture>> texture_pool;
public:
    int load_texture(const std::string& path) {
        if(path.empty()) return -1; // 空路径无法加载
        if (texture_map.count(path)) return texture_map[path]; // 已加载过，直接返回id

        texture_map[path] = next_id;
        texture_pool.push_back(std::make_unique<Texture>(path));

        std::cout << "Texture loaded: " << path << " (ID: " << next_id << ")" << std::endl;
        return next_id++;
    }
        
    Texture* get_texture(int id) const {
        assert(id >= 0 && id < texture_pool.size());
        return texture_pool[id].get();
    }
};

struct MaterialParameters {
    vec3 diffuse_color = {1.f, 1.f, 1.f};
    vec3 ambient = {0.1f, 0.1f, 0.1f};
    vec3 diffuse = {0.4f, 0.4f, 0.4f};
    vec3 specular = {0.7f, 0.7f, 0.7f};
    float shininess = 64.f;
};

struct Material {
    MaterialParameters params;
    std::string shader_id = "standard"; // 默认使用 standard 着色器

    /* decouple material, texture*/ 
    int diffuse_tex_id = -1; 
    int normal_tex_id  = -1;
    int nm_tangent_tex_id = -1;
    int specular_tex_id = -1;

    enum Feature {
        USE_DIFFUSE_MAP = 1 << 0,
        USE_NORMAL_MAP  = 1 << 1,
        USE_SPECULAR_MAP = 1 << 2,
        USE_NM_TANGENT_MAP = 1 << 3
    };
    int features = 0; // 默认不开启任何纹理

    bool has_feature(Feature f) const { return features & f; }
};

class MaterialManager {
private:
    int next_id = 0; // 将要分配的材质id，从0开始递增
    std::vector<std::unique_ptr<Material>> material_pool;
public:
    int add_material(const Material& mtl) {
        material_pool.push_back(std::make_unique<Material>(mtl));
        return next_id++;
    }
        
    Material* get_material(int id) const {
        assert(id >= 0 && id < material_pool.size());
        return material_pool[id].get();
    }
};
