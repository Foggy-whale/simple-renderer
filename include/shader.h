#pragma once
#include <vector>
#include <map>
#include <memory>
#include "geometry.h"
#include "model.h"
#include "light.h"
#include "global.h"

/* 定义Vertex类 */
struct Vertex {
    vec4 pos;
    vec3 world_pos; // Need world pos for lighting calculation
    vec3 color;
    vec2 uv;

    /* 切线空间-TBN */
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
};

/* 定义ShaderContext结构体，用于分离Shader的上下文和方法 */
struct ShaderContext {
    /* 全局参数 */
    mat4 vp; // projection * view
    vec3 eye_pos;
    TextureManager* texMgr;
    const std::vector<Light>* lights;

    /* 模型参数 */
    mat4 model;
    const Material* mtl;

    mat4 mvp; // projection * view * model
};

/* 定义IShader抽象类 */
class IShader {
protected:
    ShaderContext* context = nullptr;
    
    vec4 get_diffuse_color(const vec2& uv) const;
    vec3 get_specular_color(const vec2& uv) const;
    vec3 compute_lighting(const vec3& point, const vec3& normal, const vec3& diffuse_color, const vec3& specular_color, const vec3& ka, const vec3& kd, const vec3& ks, float p);
public:
    void bind_context(ShaderContext* ctx) { context = ctx; }

    virtual Vertex vertex(const Mesh& mesh, int iface, int nthvert) = 0;
    virtual bool fragment(const Vertex& v, vec4& rgba) = 0;
};

/* 定义ShaderManager类 */
class ShaderManager {
private:
    std::unordered_map<std::string, std::unique_ptr<IShader>> shader_pool;
public:
    void register_shader(const std::string& name, std::unique_ptr<IShader> s) {
        shader_pool[name] = std::move(s);
    }

    IShader* get_shader(const std::string& name) {
        if (shader_pool.count(name)) {
            return shader_pool[name].get();
        }
        return nullptr;
    }
};

/* 定义FlatShader类 */
class FlatShader : public IShader {
private:
    // Per-face logic to store face normal
    int last_face_idx = -1;
    vec3 face_normal;
    vec3 face_point;
public:
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

/* 定义GouraudShader类 */
class GouraudShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

/* 定义PhongShader类 */
class PhongShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

/* 定义NormalShader类 */
class NormalShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

class StandardShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

/* TODO: 还需要后续完善 EyeShader */
class EyeShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};
