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
    std::string line;
    int current_vert_offset = verts.size(); // Offset for vertex indices
    
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                // f is 1-based index from the file.
                // --f makes it 0-based relative to the *current* file.
                // We need to add the offset of existing vertices.
                facet_vrt.push_back((--f) + current_vert_offset);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated: " << filename << std::endl;
                // return; 
            }
        }
    }
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
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;
}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size()/3; }

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}
