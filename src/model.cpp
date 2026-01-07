#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include "model.h"

namespace fs = std::filesystem;

void Model::load_obj(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    Mesh mesh;
    mesh.name = filename;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            mesh.verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec3 n;
            for (int i : {0,1,2}) iss >> n[i];
            mesh.norms.push_back(n);
        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            std::array<int, 3> face_indices;
            std::array<int, 3> face_norm_indices;
            while (iss >> f >> trash >> t >> trash >> n) {
                if (cnt < 3) {
                    face_indices[cnt] = --f;
                    face_norm_indices[cnt] = --n;
                }
                cnt++;
            }
            if (3==cnt) {
                mesh.facet_vrt.push_back(face_indices);
                mesh.facet_nrm.push_back(face_norm_indices);
            } else {
                std::cerr << "Error: the obj file is supposed to be triangulated: " << filename << std::endl;
            }
        }
    }
    meshes.push_back(mesh);
}

Model::Model(const std::string filename) {
    if (fs::is_directory(filename)) {
        for (const auto& entry : fs::directory_iterator(filename)) {
            if (entry.path().extension() == ".obj") {
                load_obj(entry.path().string());
            }
        }
    } else {
        load_obj(filename);
    }
    std::cerr << "# meshes " << meshes.size() << std::endl;
}

int Model::nmeshes() const {
    return meshes.size();
}

Mesh& Model::mesh(int i) {
    return meshes[i];
}

const Mesh& Model::mesh(int i) const {
    return meshes[i];
}

mat4 Model::get_model_matrix() const {
    mat4 model = identity<4>(), translate, scale, rotateX, rotateY, rotateZ;
    translate << 1, 0, 0, pos.x,
                 0, 1, 0, pos.y,
                 0, 0, 1, pos.z,
                 0, 0, 0, 1;

    scale << scl.x, 0, 0, 0,
             0, scl.y, 0, 0,
             0, 0, scl.z, 0,
             0, 0, 0, 1;

    rotateX << std::cos(rot.x), -std::sin(rot.x), 0, 0,
              std::sin(rot.x), std::cos(rot.x), 0, 0,
              0, 0, 1, 0,
              0, 0, 0, 1;
    rotateY << std::cos(rot.y), 0, -std::sin(rot.y), 0,
              0, 1, 0, 0,
              std::sin(rot.y), 0, std::cos(rot.y), 0,
              0, 0, 0, 1;
    rotateZ << 1, 0, 0, 0,
              0, std::cos(rot.z), -std::sin(rot.z), 0,
              0, std::sin(rot.z), std::cos(rot.z), 0,
              0, 0, 0, 1;

    model = translate * rotateX * rotateY * rotateZ * scale; // ZYX order
    return model;
}
