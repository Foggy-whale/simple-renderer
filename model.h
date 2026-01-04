#pragma once
#include <vector>
#include <string>
#include <array>
#include "geometry.h"

struct Mesh {
    std::string name;
    std::vector<vec3> verts; // array of vertices
    std::vector<std::array<int, 3>> facet_vrt; // per-triangle index in the above array
    
    // std::vector<vec3> norms; // per-vertex normals
    // std::vector<vec2> uv;    // per-vertex texture coordinates
};

class Model {
    vec3 pos, scl, rot; // position, scale, rotation serve for model transformation
    std::vector<Mesh> meshes;
    void load_obj(const std::string filename);
public:
    Model() = default;
    Model(const std::string filename);
    
    int nmeshes() const;
    Mesh& mesh(int i);
    const Mesh& mesh(int i) const;
    
    Model& set_pos(vec3 pos);
    Model& set_scale(vec3 scale);
    Model& set_rot(vec3 rot);
    mat4 get_model_matrix() const;
}; 
