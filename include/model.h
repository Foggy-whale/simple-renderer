#pragma once
#include <vector>
#include <string>
#include <array>
#include "geometry.h"
#include "global.h"
#include "texture.h"

struct Mesh {
    std::string name;
    int material_id = -1; // decouple mesh and material

    std::vector<vec3> verts; // array of vertices
    std::vector<std::array<int, 3>> facet_vrt; // per-triangle index in the above array
    
    std::vector<vec3> norms; // per-vertex normals
    std::vector<std::array<int, 3>> facet_nrm; // per-triangle normal indices

    std::vector<vec2> uvs;    // per-vertex texture coordinates
    std::vector<std::array<int, 3>> facet_uv; // per-triangle uv indice
    
    std::vector<vec3> tangents; // per-vertex tangents
};

class Model {
friend class ModelManager;
private:
    vec3 min_pos = {FLOAT_MAX, FLOAT_MAX, FLOAT_MAX};
    vec3 max_pos = {FLOAT_MIN, FLOAT_MIN, FLOAT_MIN};
    std::vector<Mesh> meshes;
public:
    Model() = default;
   
    int nmeshes() const { return meshes.size(); }
    Mesh& mesh(int i) { return meshes[i]; }
    const Mesh& mesh(int i) const { return meshes[i]; }

    void add_mesh(const Mesh& m) { meshes.push_back(std::move(m)); }
};

class ModelManager {
private:
    int next_id = 0;
    std::vector<std::unique_ptr<Model>> model_pool;
public:
    int create_empty_model() {
        model_pool.push_back(std::make_unique<Model>());
        return next_id++;
    }
    Mesh& load_obj_to_model(int model_id, const std::string &full_path);

    Model* get_model(int id) { return model_pool[id].get(); }
};

class Entity {
friend class EntityManager;
private:
    int model_id;      // 指向 ModelManager 中的 ID
    vec3 pos, rot, scl; // 每个实例特有的变换属性
public:
    Entity(int m_id) : model_id(m_id), pos(0, 0, 0), rot(0, 0, 0), scl(1, 1, 1) {}
    
    Entity& set_pos(const vec3& p) { pos = p; return *this; }
    Entity& set_rot(const vec3& r) { rot = r; return *this; }
    Entity& set_scale(const vec3& s) { scl = s; return *this; }

    mat4 get_matrix() const;
    int get_model_id() const { return model_id; }
};

class EntityManager {
private:
    std::vector<std::unique_ptr<Entity>> entities;
public:
    Entity& create_entity(int model_id) {
        entities.push_back(std::make_unique<Entity>(model_id));
        return *entities.back();
    }
    const std::vector<std::unique_ptr<Entity>>& get_entities() const { return entities; }
    
    void clear() { entities.clear(); }
};
