#pragma once
#include <vector>
#include <memory>
#include "geometry.h"
#include "model.h"

struct Light {
    vec3 position;
    vec3 intensity;
};

struct Vertex {
    vec4 pos;
    vec3 world_pos; // Need world pos for lighting calculation
    vec3 color;
    vec3 normal;
    vec2 uv;
};

struct IShader {
    virtual ~IShader() = default;
    virtual Vertex vertex(const Mesh& mesh, int iface, int nthvert) = 0;
    virtual bool fragment(const Vertex& v, vec3& color) = 0;
};

struct FlatShader : public IShader {
    mat4 mvp;
    mat4 model; // Needed for world pos
    std::vector<Light> lights;
    vec3 eye_pos; // Needed for specular
    
    // Per-face logic to store face normal
    int last_face_idx = -1;
    vec3 face_normal;
    vec3 face_point;
    
    // Material properties
    vec3 ka = {0.05f, 0.05f, 0.05f};
    // vec3 ks = {0.3f, 0.3f, 0.3f};
    vec3 ks = {0.7937f, 0.7937f, 0.7937f};
    // vec3 amb_light_intensity = {0.f, 0.f, 0.f};
    vec3 amb_light_intensity = {3.f, 3.f, 3.f};
    float p = 150.f;

    vec3 compute_lighting(const vec3& point, const vec3& normal);
    Vertex vertex(const Mesh& mesh, int iface, int nthvert) override;
    bool fragment(const Vertex& v, vec3& color) override;
};
