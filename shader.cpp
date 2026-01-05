#include "shader.h"
#include <algorithm>
#include <cmath>

vec3 FlatShader::compute_lighting(const vec3& point, const vec3& normal) {
    vec3 result_color = {0, 0, 0};
    vec3 kd = {0.4f, 0.3f, 0.2f};
    // vec3 kd = {0.58f, 0.47f, 0.36f}; // Fixed kd

    for (const auto& light : lights) {
        vec3 l = (light.position - point).normalized();
        vec3 v_dir = (eye_pos - point).normalized();
        vec3 h = (l + v_dir).normalized();
        float r_sq = (light.position - point).norm();
        r_sq = r_sq * r_sq;
        
        vec3 I = light.intensity;

        vec3 La = {ka.x * amb_light_intensity.x, ka.y * amb_light_intensity.y, ka.z * amb_light_intensity.z};
        
        float diff = std::max(0.f, normal * l);
        vec3 Ld = {kd.x * I.x / r_sq * diff, kd.y * I.y / r_sq * diff, kd.z * I.z / r_sq * diff};
        
        float spec = std::pow(std::max(0.f, normal * h), p);
        vec3 Ls = {ks.x * I.x / r_sq * spec, ks.y * I.y / r_sq * spec, ks.z * I.z / r_sq * spec};
        
        result_color += La + Ld + Ls;
    }
    
    return result_color;
}

Vertex FlatShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = mvp * to_vec4(vertex_pos);
    static vec3 color = {0.0f, 0.0f, 0.0f}; 
    
    // Calculate per-face normal and representative point once per face
    if (iface != last_face_idx) {
        // 1. Calculate world space coords
        vec3 p0 = (model * to_vec4(mesh.verts[mesh.facet_vrt[iface][0]])).xyz();
        vec3 p1 = (model * to_vec4(mesh.verts[mesh.facet_vrt[iface][1]])).xyz();
        vec3 p2 = (model * to_vec4(mesh.verts[mesh.facet_vrt[iface][2]])).xyz();
        
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
