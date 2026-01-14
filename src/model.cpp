#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include "model.h"

namespace fs = std::filesystem;

static void calculate_mesh_tangents(Mesh& mesh) {
    mesh.tangents.resize(mesh.verts.size(), vec3(0, 0, 0));

    for (int i = 0; i < mesh.facet_vrt.size(); i++) {
        int idx0 = mesh.facet_vrt[i][0];
        int idx1 = mesh.facet_vrt[i][1];
        int idx2 = mesh.facet_vrt[i][2];

        // 获取顶点坐标与UV坐标
        vec3& v0 = mesh.verts[idx0];
        vec3& v1 = mesh.verts[idx1];
        vec3& v2 = mesh.verts[idx2];
        vec2& uv0 = mesh.uvs[mesh.facet_uv[i][0]];
        vec2& uv1 = mesh.uvs[mesh.facet_uv[i][1]];
        vec2& uv2 = mesh.uvs[mesh.facet_uv[i][2]];

        // 计算边向量和UV差值向量
        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;
        float du1 = uv1.x - uv0.x;
        float dv1 = uv1.y - uv0.y;
        float du2 = uv2.x - uv0.x;
        float dv2 = uv2.y - uv0.y;

        // 用Cramer法则计算切线T
        float inv = 1.0f / (du1 * dv2 - du2 * dv1);
        vec3 tangent;
        tangent.x = inv * (dv2 * edge1.x - dv1 * edge2.x);
        tangent.y = inv * (dv2 * edge1.y - dv1 * edge2.y);
        tangent.z = inv * (dv2 * edge1.z - dv1 * edge2.z);

        // 合成向量以平滑切线
        mesh.tangents[idx0] += tangent;
        mesh.tangents[idx1] += tangent;
        mesh.tangents[idx2] += tangent;
    }

    // 最后对所有切线进行归一化
    for (int i = 0; i < mesh.tangents.size(); i++) {
        mesh.tangents[i] = mesh.tangents[i].normalized();
    }
}

Mesh& ModelManager::load_obj_to_model(int model_id, const std::string &full_path) {
    std::ifstream in(full_path);
    if (in.fail()) {
        std::cerr << "Failed to open OBJ: " << full_path << std::endl;
        exit(-1);
    }

    Model* model = get_model(model_id);
    Mesh mesh;
    mesh.name = std::filesystem::path(full_path).stem().string();

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            vec3 v; iss >> v.x >> v.y >> v.z;
            mesh.verts.push_back(v);
            // 更新模型包围盒
            model->min_pos = min(model->min_pos, v);
            model->max_pos = max(model->max_pos, v);
        } else if (prefix == "vn") {
            vec3 n; iss >> n.x >> n.y >> n.z;
            mesh.norms.push_back(n);
        } else if (prefix == "vt") {
            vec2 uv; iss >> uv.x >> uv.y;
            mesh.uvs.push_back(uv);
        } else if (prefix == "f") {
            std::array<int, 3> f, t, n;
            char trash;
            for (int i = 0; i < 3; i++) {
                iss >> f[i] >> trash >> t[i] >> trash >> n[i];
                f[i]--; t[i]--; n[i]--;
            }
            mesh.facet_vrt.push_back(f);
            mesh.facet_uv.push_back(t);
            mesh.facet_nrm.push_back(n);
        }
    }
    calculate_mesh_tangents(mesh);
    model->add_mesh(std::move(mesh));
    return model->meshes.back();
}

mat4 Entity::get_matrix() const {
    mat4 model = identity<4>(), translate, scale, rotateX, rotateY, rotateZ;
    float rad_x = rot.x * M_PI / 180.0,
          rad_y = rot.y * M_PI / 180.0,
          rad_z = rot.z * M_PI / 180.0;

    translate << 1, 0, 0, pos.x,
                 0, 1, 0, pos.y,
                 0, 0, 1, pos.z,
                 0, 0, 0, 1;

    scale << scl.x, 0, 0, 0,
             0, scl.y, 0, 0,
             0, 0, scl.z, 0,
             0, 0, 0, 1;

    rotateX << 1, 0, 0, 0,
               0, std::cos(rad_x), -std::sin(rad_x), 0,
               0, std::sin(rad_x), std::cos(rad_x), 0,
               0, 0, 0, 1;

    rotateY << std::cos(rad_y), 0, std::sin(rad_y), 0,
               0, 1, 0, 0,
               -std::sin(rad_y), 0, std::cos(rad_y), 0,
               0, 0, 0, 1;

    rotateZ << std::cos(rad_z), -std::sin(rad_z), 0, 0,
               std::sin(rad_z), std::cos(rad_z), 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1;

    model = translate * rotateX * rotateY * rotateZ * scale;
    return model;
}