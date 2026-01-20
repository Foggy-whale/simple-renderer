#pragma once
#include <vector>
#include <map>
#include <memory>
#include "geometry.h"
#include "triangle.h"
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
    
    // 静态辅助接口：对 Vertex 属性进行插值
    // 注意：pos 比较特殊，不在这里做插值
    static Vertex lerp(float alpha, float beta, float gamma, const Triangle& t) {
        Vertex v;

        v.color     = t.color[0] * alpha     + t.color[1] * beta     + t.color[2] * gamma;
        v.uv        = t.tex_coord[0] * alpha + t.tex_coord[1] * beta + t.tex_coord[2] * gamma;
        v.world_pos = t.world_pos[0] * alpha + t.world_pos[1] * beta + t.world_pos[2] * gamma;
        v.normal    = (t.normal[0] * alpha   + t.normal[1] * beta    + t.normal[2] * gamma).normalized();
        v.tangent   = (t.tangent[0] * alpha   + t.tangent[1] * beta   + t.tangent[2] * gamma).normalized();
        v.bitangent = (t.bitangent[0] * alpha + t.bitangent[1] * beta + t.bitangent[2] * gamma).normalized();

        return v;
    }
};

/* 定义ShadowMapData结构体，用于存储阴影贴图数据 */
struct ShadowMapData {
    std::vector<float> buffer; // 深度缓冲区
    mat4 light_vp;           // 光源 View-Projection 矩阵
};

class IShadowStrategy; // 前向声明阴影策略接口

/* 定义ShaderContext结构体，用于分离Shader的上下文和方法 */
struct ShaderContext {
    /* 全局参数 */
    mat4 vp; // projection * view
    vec3 eye_pos;
    TextureManager* texMgr;
    const std::vector<Light>* lights;

    /* 阴影贴图数据 */
    std::vector<ShadowMapData> shadow_datas;
    std::unique_ptr<IShadowStrategy> shadow_strategy; // 注入阴影算法

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

class DepthShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec4& rgba) override;
};

/* 定义IShadowStrategy抽象类，用于阴影计算 */
class IShadowStrategy {
protected:
    float sample_buffer(const std::vector<float>& buffer, vec2 uv) {
        int x = std::clamp((int)(uv.x * sm_width), 0, sm_width - 1);
        int y = std::clamp((int)(uv.y * sm_height), 0, sm_height - 1);
        float val = buffer[x + y * sm_width];
        return val;
    }
    float sample_buffer_bilinear(const std::vector<float>& buffer, vec2 uv) {
        float u = uv.x * (sm_width - 1);
        float v = uv.y * (sm_height - 1);

        int x0 = (int)std::floor(u);
        int y0 = (int)std::floor(v);
        int x1 = std::clamp(x0 + 1, 0, sm_width - 1);
        int y1 = std::clamp(y0 + 1, 0, sm_height - 1);

        float s = u - x0;
        float t = v - y0;

        float d00 = buffer[x0 + y0 * sm_width];
        float d10 = buffer[x1 + y0 * sm_width];
        float d01 = buffer[x0 + y1 * sm_width];
        float d11 = buffer[x1 + y1 * sm_width];

        float lerp_top = d00 + s * (d10 - d00);
        float lerp_bottom = d01 + s * (d11 - d01);
        
        return lerp_top + t * (lerp_bottom - lerp_top);
    }
public:
    virtual ~IShadowStrategy() = default;

    // 返回值 0.0~1.0，表示光照强度系数
    virtual float calculate_shadow(int light_idx, const vec3& world_pos, const vec3 &normal, const ShaderContext* context) = 0;
};

/* 定义HardShadowStrategy类，用于硬阴影计算 */
class HardShadowStrategy : public IShadowStrategy {
public:
    float calculate_shadow(int light_idx, const vec3& world_pos, const vec3 &normal, const ShaderContext* context) override;
};

// 高级阴影：PCSS (为后续实现预留)
class PCSSShadowStrategy : public IShadowStrategy {
public:
    float calculate_shadow(int light_idx, const vec3& world_pos, const vec3 &normal, const ShaderContext* context) override;
};