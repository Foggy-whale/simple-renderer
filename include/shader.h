#pragma once
#include <vector>
#include <map>
#include <memory>
#include "geometry.h"
#include "material.h"
#include "model.h"
#include "global.h"

/* 定义Vertex类 */
struct Vertex {
    vec4 pos;
    vec3 world_pos; // Need world pos for lighting calculation
    vec3 color;
    vec3 normal;
    vec2 uv;
};

/* 定义IShader抽象类 */
class IShader {
protected:
    mat4 mvp;
    mat4 model; // Needed for world pos
    vec3 eye_pos; // Needed for specular
    std::vector<Light> lights;
    MaterialParameters mat_props;

    vec3 compute_lighting(const vec3& point, const vec3& normal);

public:
    virtual ~IShader() = default;

    IShader& set_mvp(const mat4& mvp_in) { mvp = mvp_in; return *this; }
    IShader& set_model(const mat4& model_in) { model = model_in; return *this; }
    IShader& set_eye_pos(const vec3& eye_pos_in) { eye_pos = eye_pos_in; return *this; }
    IShader& set_lights(const std::vector<Light>& lights_in) { lights = lights_in; return *this; }
    IShader& set_mat_props(const MaterialParameters& mat_props_in) { mat_props = mat_props_in; return *this; }
    
    virtual Vertex vertex(const Mesh& mesh, int iface, int nthvert) = 0;
    virtual bool fragment(const Vertex& v, vec3& color) = 0;
};

/* 定义ShaderManager类 */
class ShaderManager {
private:
    std::map<std::string, std::shared_ptr<IShader>> shader_pool;

public:
    void register_shader(const std::string& name, std::shared_ptr<IShader> s) {
        shader_pool[name] = s;
    }

    IShader* get_shader(const std::string& name) {
        if (shader_pool.count(name)) {
            return shader_pool[name].get(); // 返回原始指针，不增加引用计数
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
    bool fragment(const Vertex& v, vec3& color) override;
};

/* 定义GouraudShader类 */
class GouraudShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec3& color) override;
};

/* 定义PhongShader类 */
class PhongShader : public IShader {
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec3& color) override;
};
