#include "shader.h"
#include <algorithm>
#include <cmath>

vec3 IShader::compute_lighting(const vec3& point, const vec3& normal) {
    vec3 result_color = {0, 0, 0};
    vec3 amb_light_intensity = {3.f, 3.f, 3.f};
    vec3 diffuse_color = mat_props.diffuse_color;
    vec3 ka = mat_props.ambient;
    vec3 kd = mat_props.diffuse;
    vec3 ks = mat_props.specular;
    float p = mat_props.shininess;

    for (const auto& light : lights) {
        vec3 l = (light.position - point).normalized();
        vec3 v_dir = (eye_pos - point).normalized();
        vec3 h = (l + v_dir).normalized();
        float r_sq = (light.position - point).norm();
        r_sq = r_sq * r_sq;
        
        vec3 I = light.intensity;

        vec3 La = ka * amb_light_intensity;
        
        float diff = std::max(0.f, dot_product(normal, l));
        vec3 Ld = kd * diffuse_color * I / r_sq * diff;
        
        float spec = std::pow(std::max(0.f, dot_product(normal, h)), p);
        vec3 Ls = ks * I / r_sq * spec;
        
        result_color += La + Ld + Ls;
    }
    
    result_color.x = std::min(1.f, result_color.x);
    result_color.y = std::min(1.f, result_color.y);
    result_color.z = std::min(1.f, result_color.z);

    return result_color;
}

Vertex FlatShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = mvp * embed<4>(vertex_pos, 1.f);
    static vec3 color = {0.0f, 0.0f, 0.0f}; 
    
    // Calculate per-face normal and representative point once per face
    if (iface != last_face_idx) {
        // 1. Calculate world space coords
        vec3 p0 = (model * embed<4>(mesh.verts[mesh.facet_vrt[iface][0]], 1.f)).xyz();
        vec3 p1 = (model * embed<4>(mesh.verts[mesh.facet_vrt[iface][1]], 1.f)).xyz();
        vec3 p2 = (model * embed<4>(mesh.verts[mesh.facet_vrt[iface][2]], 1.f)).xyz();
        
        // 2. Face normal
        face_normal = cross_product(p1 - p0, p2 - p0).normalized();
        
        // 3. Centroid
        vec3 centroid = (p0 + p1 + p2) / 3.0f;

        // 4. Compute lighting once per face
        color = compute_lighting(centroid, face_normal); 

        last_face_idx = iface;
    }
    
    // Use calculated face color
    v.color = color; 
    
    return v;
}

bool FlatShader::fragment(const Vertex& v, vec3& color) {
    color = v.color; // 直接使用顶点传过来的颜色（由于三个顶点颜色相同，插值结果也是恒定的）
    return false;
}

// Gouraud Shader Implementation
Vertex GouraudShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = mvp * embed<4>(vertex_pos, 1.f);
    
    vec3 world_pos = (model * embed<4>(vertex_pos, 1.f)).xyz();
    v.world_pos = world_pos;
    
    vec3 normal = mesh.norms[mesh.facet_nrm[iface][nthvert]];
    mat<4> normal_matrix = model.inverse_transpose();
    vec3 world_normal = (normal_matrix * embed<4>(normal)).xyz().normalized();
    v.normal = world_normal;
    
    v.color = compute_lighting(world_pos, world_normal);
    v.uv = {0, 0};
    return v;
}

bool GouraudShader::fragment(const Vertex& v, vec3& color) {
    color = v.color;
    return false;
}

// Phong Shader Implementation
Vertex PhongShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = mvp * embed<4>(vertex_pos, 1.f);
    
    v.world_pos = (model * embed<4>(vertex_pos, 1.f)).xyz();
    
    vec3 normal = mesh.norms[mesh.facet_nrm[iface][nthvert]];
    mat<4> normal_matrix = model.inverse_transpose();
    vec3 world_normal = (normal_matrix * embed<4>(normal)).xyz().normalized();
    v.normal = world_normal;
    
    v.color = {0, 0, 0}; // Not used
    v.uv = {0, 0};
    return v;
}

bool PhongShader::fragment(const Vertex& v, vec3& color) {
    vec3 n = v.normal.normalized();
    color = compute_lighting(v.world_pos, n);
    return false;
}
