#include "shader.h"
#include <algorithm>
#include <cmath>

vec4 IShader::get_diffuse_color(const vec2& uv) const {
    float alpha = 1.f;
    vec3 result_color = context->mtl->params.diffuse_color;
    if(context->mtl->has_feature(Material::USE_DIFFUSE_MAP)) {
        Texture* diffuse_map = context->texMgr->get_texture(context->mtl->diffuse_tex_id);
        assert(diffuse_map && "Diffuse map is nullptr");
        TGAColor diffuse_color = diffuse_map->sample_uv(uv);
        result_color =  result_color * vec3(diffuse_color[2], diffuse_color[1], diffuse_color[0]) / 255.f;
        alpha = diffuse_color.bytespp == 4 ? diffuse_color[3] / 255.f : 1.f;
    }
    return embed<4>(result_color, alpha);
}

vec3 IShader::get_specular_color(const vec2& uv) const {
    vec3 result_color = vec3(1.f, 1.f, 1.f);
    if(context->mtl->has_feature(Material::USE_SPECULAR_MAP)) {
        Texture* specular_map = context->texMgr->get_texture(context->mtl->specular_tex_id);
        assert(specular_map && "Specular map is nullptr");
        TGAColor specular_color = specular_map->sample_uv(uv);
        result_color =  result_color * vec3(specular_color[0], specular_color[0], specular_color[0]) / 255.f;
    }
    return result_color;
}


vec3 IShader::compute_lighting(const vec3& point, const vec3& normal, const vec3& diffuse_color, const vec3& specular_color, const vec3& ka, const vec3& kd, const vec3& ks, float p) {
    vec3 result_color = {0, 0, 0};
    
    for(int light_idx = 0; light_idx < context->lights->size(); light_idx++) {
        const auto& light = (*context->lights)[light_idx];

        // 计算光照方向和半程向量
        vec3 l = (light.position - point).normalized();
        vec3 v = (context->eye_pos - point).normalized();
        vec3 h = (l + v).normalized();
        float r_sq = (light.position - point).norm();
        r_sq = r_sq * r_sq;
        
        // 计算阴影可见性
        float shadow_visibility = 1.0f;
        if(context->shadow_strategy) {
            shadow_visibility = context->shadow_strategy->calculate_shadow(light_idx, point, normal, context);
        }

        // 计算光照强度
        vec3 I = light.intensity;

        vec3 La = ka * diffuse_color;
        
        float diff = std::max(0.f, dot_product(normal, l));
        vec3 Ld = kd * I / r_sq * diff * diffuse_color;
        
        float spec = std::pow(std::max(0.f, dot_product(normal, h)), p);
        vec3 Ls = ks * I / r_sq * spec * specular_color;
        
        result_color += La + (Ld + Ls) * shadow_visibility;
    }
    
    return result_color;
}

Vertex FlatShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);

    static vec3 color = {0.0f, 0.0f, 0.0f}; 
    
    // Calculate per-face normal and representative point once per face
    if (iface != last_face_idx) {
        // 1. Calculate world space coords
        vec3 p0 = (context->model * embed<4>(mesh.verts[mesh.facet_vrt[iface][0]], 1.f)).xyz();
        vec3 p1 = (context->model * embed<4>(mesh.verts[mesh.facet_vrt[iface][1]], 1.f)).xyz();
        vec3 p2 = (context->model * embed<4>(mesh.verts[mesh.facet_vrt[iface][2]], 1.f)).xyz();
        
        // 2. Face normal
        face_normal = cross_product(p1 - p0, p2 - p0).normalized();
        
        // 3. Centroid
        vec3 centroid = (p0 + p1 + p2) / 3.0f;

        // 4. Compute lighting once per face
        color = compute_lighting(centroid, face_normal, vec3(1.f, 1.f, 1.f), vec3(1.f, 1.f, 1.f), context->mtl->params.ambient, context->mtl->params.diffuse, context->mtl->params.specular, context->mtl->params.shininess); 

        last_face_idx = iface;
    }
    
    // Use calculated face color
    v.color = color; 
    
    return v;
}

bool FlatShader::fragment(const Vertex& v, vec4& rgba) {
    rgba = embed<4>(v.color, 1.f); // 直接使用顶点传过来的颜色（由于三个顶点颜色相同，插值结果也是恒定的）
    return false;
}

// Gouraud Shader Implementation
Vertex GouraudShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    int idx_vert = mesh.facet_vrt[iface][nthvert];
    int idx_norm = mesh.facet_nrm[iface][nthvert];

    vec3 vertex_pos = mesh.verts[idx_vert];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);
    
    vec3 world_pos = (context->model * embed<4>(vertex_pos, 1.f)).xyz();
    v.world_pos = world_pos;
    
    vec3 normal = mesh.norms[idx_norm];
    mat<4> normal_matrix = context->model.inverse_transpose();
    v.normal = (normal_matrix * embed<4>(normal)).xyz().normalized();
    
    v.color = compute_lighting(world_pos, v.normal, vec3(1.f, 1.f, 1.f), vec3(1.f, 1.f, 1.f), 
                                context->mtl->params.ambient, 
                                context->mtl->params.diffuse, 
                                context->mtl->params.specular, 
                                context->mtl->params.shininess);
    v.uv = {0, 0};
    return v;
}

bool GouraudShader::fragment(const Vertex& v, vec4& rgba) {
    rgba = embed<4>(v.color, 1.f);
    return false;
}

// Phong Shader Implementation
Vertex PhongShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    int idx_vert = mesh.facet_vrt[iface][nthvert];
    int idx_norm = mesh.facet_nrm[iface][nthvert];
    
    vec3 vertex_pos = mesh.verts[idx_vert];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);
    v.world_pos = (context->model * embed<4>(vertex_pos, 1.f)).xyz();
    
    vec3 normal = mesh.norms[idx_norm];
    mat<4> normal_matrix = context->model.inverse_transpose();
    v.normal = (normal_matrix * embed<4>(normal)).xyz().normalized();
    
    v.color = {0, 0, 0}; // Not used
    v.uv = {0, 0}; // Not used
    return v;
}

bool PhongShader::fragment(const Vertex& v, vec4& rgba) {
    vec3 n = v.normal.normalized();
    vec3 color = compute_lighting(v.world_pos, n, vec3(1.f, 1.f, 1.f), vec3(1.f, 1.f, 1.f), 
                                    context->mtl->params.ambient, 
                                    context->mtl->params.diffuse,
                                    context->mtl->params.specular, 
                                    context->mtl->params.shininess);
    rgba = embed<4>(color, 1.f);
    return false;
}

Vertex NormalShader::vertex(const Mesh &mesh, int iface, int nthvert) {
    Vertex v;
    int idx_vert = mesh.facet_vrt[iface][nthvert];
    int idx_uv = mesh.facet_uv[iface][nthvert];

    vec3 vertex_pos = mesh.verts[idx_vert];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);
    v.world_pos = (context->model * embed<4>(vertex_pos, 1.f)).xyz();

    v.uv = mesh.uvs[idx_uv];

    return v;
}

bool NormalShader::fragment(const Vertex& v, vec4& rgba) {
    Texture* normal_map = context->texMgr->get_texture(context->mtl->normal_tex_id);
    assert(normal_map && "Normal map is nullptr");
    
    TGAColor normal_color = normal_map->sample_uv(v.uv);
    vec3 n = vec3(normal_color[2], normal_color[1], normal_color[0]) / 255.f * 2.f - vec3(1, 1, 1);

    vec3 color = compute_lighting(v.world_pos, n.normalized(), vec3(1.f, 1.f, 1.f), vec3(1.f, 1.f, 1.f), 
                                    context->mtl->params.ambient, 
                                    context->mtl->params.diffuse, 
                                    context->mtl->params.specular, 
                                    context->mtl->params.shininess);
    rgba = embed<4>(color, 1.f);
    return false;
}

Vertex StandardShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    int idx_vert = mesh.facet_vrt[iface][nthvert];
    int idx_uv = mesh.facet_uv[iface][nthvert];
    int idx_norm = mesh.facet_nrm[iface][nthvert];

    vec3 vertex_pos = mesh.verts[idx_vert];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);
    v.world_pos = (context->model * embed<4>(vertex_pos, 1.f)).xyz();

    v.uv = mesh.uvs[idx_uv];

    vec3 normal = mesh.norms[idx_norm];
    mat<4> normal_matrix = context->model.inverse_transpose();
    v.normal = (normal_matrix * embed<4>(normal)).xyz().normalized();

    if(context->mtl->has_feature(Material::USE_NM_TANGENT_MAP)) {
        v.tangent = (context->model * embed<4>(mesh.tangents[idx_vert])).xyz().normalized();

        // 施密特正交化保证「切线」与「法线」垂直!
        v.tangent = (v.tangent - v.normal * dot_product(v.tangent, v.normal)).normalized();
        v.bitangent = cross_product(v.normal, v.tangent).normalized();
    }

    return v;
}

bool StandardShader::fragment(const Vertex& v, vec4& rgba) {
    vec3 n;
    if(context->mtl->has_feature(Material::USE_NORMAL_MAP)) {
        Texture* normal_map = context->texMgr->get_texture(context->mtl->normal_tex_id);
        assert(normal_map && "Normal map is nullptr");

        TGAColor normal_color = normal_map->sample_uv(v.uv);
        n = vec3(normal_color[2], normal_color[1], normal_color[0]) / 255.f * 2.f - vec3(1, 1, 1);
    } 
    else if(context->mtl->has_feature(Material::USE_NM_TANGENT_MAP)) {
        Texture* tangent_map = context->texMgr->get_texture(context->mtl->nm_tangent_tex_id);
        assert(tangent_map && "Normal Tangent map is nullptr");
        
        TGAColor tangent_color = tangent_map->sample_uv(v.uv);
        n = vec3(tangent_color[2], tangent_color[1], tangent_color[0]) / 255.f * 2.f - vec3(1, 1, 1);

        // 重新规范化插值后的基向量
        vec3 normal = v.normal.normalized();
        vec3 tangent = v.tangent.normalized();
        
        // 由于做了插值，所以还要再做一次施密特正交化
        tangent = (tangent - normal * dot_product(tangent, normal)).normalized();
        vec3 bitangent = cross_product(normal, tangent).normalized();

        // 构建TBN矩阵并将法线转换到世界空间
        mat<3> tbn = mat<3>(tangent, bitangent, normal).transpose();
        n = tbn * n;
    } 
    else {
        n = v.normal;
    }
    vec4 diffuse_color = get_diffuse_color(v.uv);
    vec3 specular_color = get_specular_color(v.uv);
    
    vec3 color = compute_lighting(v.world_pos, n.normalized(), diffuse_color.xyz(), specular_color, context->mtl->params.ambient, context->mtl->params.diffuse, context->mtl->params.specular, context->mtl->params.shininess);
    rgba = embed<4>(color, diffuse_color.w);

    return false;
}

Vertex EyeShader::vertex(const Mesh& mesh, int iface, int nthvert) {
    Vertex v;
    int idx_vert = mesh.facet_vrt[iface][nthvert];
    int idx_uv = mesh.facet_uv[iface][nthvert];
    int idx_norm = mesh.facet_nrm[iface][nthvert];

    vec3 vertex_pos = mesh.verts[idx_vert];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);
    v.world_pos = (context->model * embed<4>(vertex_pos, 1.f)).xyz();

    v.uv = mesh.uvs[idx_uv];

    vec3 normal = mesh.norms[idx_norm];
    mat<4> normal_matrix = context->model.inverse_transpose();
    v.normal = (normal_matrix * embed<4>(normal)).xyz().normalized();

    if(context->mtl->has_feature(Material::USE_NM_TANGENT_MAP)) {
        v.tangent = (context->model * embed<4>(mesh.tangents[idx_vert])).xyz().normalized();

        // 施密特正交化保证「切线」与「法线」垂直!
        v.tangent = (v.tangent - v.normal * dot_product(v.tangent, v.normal)).normalized();
        v.bitangent = cross_product(v.normal, v.tangent).normalized();
    }

    return v;
}

/* TODO: 还需要后续完善 EyeShader 的 fragment 函数 */
bool EyeShader::fragment(const Vertex& v, vec4& rgba) {
    vec3 n;
    if(context->mtl->has_feature(Material::USE_NORMAL_MAP)) {
        Texture* normal_map = context->texMgr->get_texture(context->mtl->normal_tex_id);
        assert(normal_map && "Normal map is nullptr");

        TGAColor normal_color = normal_map->sample_uv(v.uv);
        n = vec3(normal_color[2], normal_color[1], normal_color[0]) / 255.f * 2.f - vec3(1, 1, 1);
    } 
    else if(context->mtl->has_feature(Material::USE_NM_TANGENT_MAP)) {
        Texture* tangent_map = context->texMgr->get_texture(context->mtl->nm_tangent_tex_id);
        assert(tangent_map && "Normal Tangent map is nullptr");
        
        TGAColor tangent_color = tangent_map->sample_uv(v.uv);
        n = vec3(tangent_color[2], tangent_color[1], tangent_color[0]) / 255.f * 2.f - vec3(1, 1, 1);

        // 重新规范化插值后的基向量
        vec3 normal = (v.normal + vec3(0, 0, 0.1f)).normalized();
        vec3 tangent = v.tangent.normalized();
        
        // 由于做了插值，所以还要再做一次施密特正交化
        tangent = (tangent - normal * dot_product(tangent, normal)).normalized();
        vec3 bitangent = cross_product(normal, tangent).normalized();

        // 构建TBN矩阵并将法线转换到世界空间
        mat<3> tbn = mat<3>(tangent, bitangent, normal).transpose();
        n = tbn * n;
    } 
    else {
        n = v.normal;
    }
    vec4 diffuse_color = get_diffuse_color(v.uv);
    vec3 specular_color = get_specular_color(v.uv);
    
    vec3 color = compute_lighting(v.world_pos, n.normalized(), diffuse_color.xyz(), specular_color, context->mtl->params.ambient, context->mtl->params.diffuse, context->mtl->params.specular, context->mtl->params.shininess);
    rgba = embed<4>(color, diffuse_color.w);

    return false;
}

Vertex DepthShader::vertex(const Mesh &mesh, int iface, int nthvert) {
    Vertex v;

    // 只做 MVP 变换，不考虑光照模型
    vec3 vertex_pos = mesh.verts[mesh.facet_vrt[iface][nthvert]];
    v.pos = context->mvp * embed<4>(vertex_pos, 1.f);

    return v;
}

bool DepthShader::fragment(const Vertex& v, vec4& rgba) {
    return false; // 仅写入深度，丢弃像素颜色
}

float HardShadowStrategy::calculate_shadow(int light_idx, const vec3 &world_pos, const vec3 &normal, const ShaderContext *context) {
    // 正面剔除对 Shadow Acne 以及 Peter Panning 效果很好，因此不用再做 Depth Bias
    const auto& sd = context->shadow_datas[light_idx];
    const auto& light = (*context->lights)[light_idx];

    vec4 light_space_pos = sd.light_vp * embed<4>(world_pos, 1.f);
    vec3 proj = light_space_pos.xyz() / light_space_pos.w;
    vec2 uv = vec2(proj.x + 1.f, proj.y + 1.f) * 0.5f;
    float z_screen = (1.f - proj.z) * 0.5f; // Reverse-Z 映射

    return (z_screen < sample_buffer(sd.buffer, uv)) ? 0.0f : 1.0f;
}

float PCSSShadowStrategy::calculate_shadow(int light_idx, const vec3& world_pos, const vec3 &normal, const ShaderContext* context) {
    const auto& sd = context->shadow_datas[light_idx];
    const auto& light = (*context->lights)[light_idx];

    vec4 light_space_pos = sd.light_vp * embed<4>(world_pos, 1.f);
    vec3 proj = light_space_pos.xyz() / light_space_pos.w;
    vec2 uv = vec2(proj.x + 1.f, proj.y + 1.f) * 0.5f;
    float z_screen = (1.f - proj.z) * 0.5f; // Reverse-Z 映射

    // Blocker Search
    float avg_blocker_depth = 0;
    int blocker_count = 0;
    float search_radius = 0.01f; // 搜索半径

    for(int i = 0; i < 16; i++) {
        float z_sample = sample_buffer_bilinear(sd.buffer, uv + poisson_disk[i] * search_radius);
        if(z_screen < z_sample) { 
            avg_blocker_depth += z_sample;
            blocker_count++;
        }
    }
    if(!blocker_count) return 1.0f;
    avg_blocker_depth /= (float)blocker_count;

    // Penumbra Estimation
    // float w_light = 0.012f; 
    float w_light = 0.025f;
    float penumbra_radius = (avg_blocker_depth - z_screen) / avg_blocker_depth * w_light;
    // penumbra_radius = std::clamp(penumbra_radius, 0.0001f, 0.04f);
    penumbra_radius = std::clamp(penumbra_radius, 0.0005f, 0.02f);

    // Filtering
    float visibility = 0.0f;
    for(int i = 0; i < 16; i++) {
        float z_sample = sample_buffer_bilinear(sd.buffer, uv + poisson_disk[i] * penumbra_radius);
        visibility += (z_screen < z_sample) ? 0.0f : 1.0f;
    }

    return visibility / 16.0f;
}
