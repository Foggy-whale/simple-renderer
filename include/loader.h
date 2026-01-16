#pragma once
#include <fstream>
#include "nlohmann/json.hpp"
#include "model.h"
#include "scene.h"
#include "shader.h"
#include "texture.h"
#include "rasterizer.h"
#include <filesystem>

using json = nlohmann::json;

struct MeshLoadConfig {
    std::string filename;
    std::string mesh_key;
    std::string shader_id;

    int feature_mask;

    MaterialParameters params;
};

class Loader {
private: 
    std::string config_path;
    std::string scene_name;
public:
    Loader() = default;
    Loader(const std::string& path, const std::string& name) : config_path(path), scene_name(name) {}

    bool load(Scene& scene, std::unique_ptr<ShaderManager>& shaderMgr,
                            std::unique_ptr<TextureManager>& texMgr, 
                            std::unique_ptr<MaterialManager>& matMgr,
                            std::unique_ptr<ModelManager>& modelMgr,
                            std::unique_ptr<EntityManager>& entityMgr) {
        std::ifstream in(config_path);
        if(in.fail()) {
            std::cerr << "Cannot open config file: " << config_path << std::endl;
            return false;
        }

        std::cout << std::endl << "=== Reading Config File ===" << std::endl;
        json data;
        try {
            data = json::parse(in);
        } catch(json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return false;
        }

        if(!data.contains(scene_name)) {
            std::cerr << "Scene '" << scene_name << "' not found in JSON." << std::endl;
            return false;
        }

        std::cout << "Succeed to read config file from: " << config_path << std::endl;
        std::cout << "=== Reading Config File End ===" << std::endl;

        auto& cfg = data[scene_name];

        // --- 解析 Camera ---
        std::cout << std::endl << "=== Parsing Camera ===" << std::endl;
        parse_camera(cfg, scene);
        std::cout << "Succeed to parse camera." << std::endl;
        std::cout << "=== Parsing Camera End ===" << std::endl;

        // --- 解析 Models ---
        std::cout << std::endl << "=== Parsing Models ===" << std::endl;
        parse_models_and_entities(cfg, scene, shaderMgr, texMgr, matMgr, modelMgr, entityMgr);
        std::cout << "Succeed to parse models." << std::endl;
        std::cout << "=== Parsing Models End ===" << std::endl;

        // --- 解析 Lights ---
        std::cout << std::endl << "=== Parsing Lights ===" << std::endl;
        parse_lights(cfg, scene);
        std::cout << "Succeed to parse lights." << std::endl;
        std::cout << "=== Parsing Lights End ===" << std::endl;

        return true;
    }

private:
    // 辅助函数：解析并添加相机
    static void parse_camera(const json& cfg, Scene& scene) {
        if(cfg.contains("camera")) {
            auto& c_cfg = cfg["camera"];
            Camera camera;
            camera.set_eye({c_cfg["eye"][0], c_cfg["eye"][1], c_cfg["eye"][2]})
                  .set_target({c_cfg["target"][0], c_cfg["target"][1], c_cfg["target"][2]})
                  .set_up({c_cfg["up"][0], c_cfg["up"][1], c_cfg["up"][2]})
                  .set_projection(45.0f, (float)width / height, 0.1f, 100.0f); 
            scene.set_camera(camera);
        } else {
            std::cerr << "Camera configuration is missing." << std::endl;
            exit(-1);
        }
    }

    // 辅助函数：加载单个网格
    static void load_single_mesh(const json& mesh_cfg, const std::string& base_path, int model_id,
                          std::unique_ptr<ModelManager>& modelMgr, std::unique_ptr<MaterialManager>& matMgr, std::unique_ptr<TextureManager>& texMgr) {
        /* 加载几何数据 */
        std::string obj_path = base_path + mesh_cfg.value("filename", "");
        Mesh& mesh = modelMgr->load_obj_to_model(model_id, obj_path);

        /* 解析材质 */
        assert(mesh_cfg.contains("material") && mesh_cfg["material"].is_object());
        auto& mat_json = mesh_cfg["material"];

        Material mtl;
        mtl.shader_id = mat_json.value("shader", "phong");

        if(mat_json.contains("feature"))
            mtl.features  = parse_features(mat_json["feature"]);
        
        if(mat_json.contains("params")) { // 如果要重载参数，就从 JSON 中读取
            auto& p = mat_json["params"];
            mtl.params.diffuse_color = {p["diffuse_color"][0], p["diffuse_color"][1], p["diffuse_color"][2]};
            mtl.params.ambient = {p["ambient"][0], p["ambient"][1], p["ambient"][2]};
            mtl.params.diffuse = {p["diffuse"][0], p["diffuse"][1], p["diffuse"][2]};
            mtl.params.specular = {p["specular"][0], p["specular"][1], p["specular"][2]};
            mtl.params.shininess = p["shininess"];
        }

        /* 加载纹理 */
        std::string tex_prefix = obj_path.substr(0, obj_path.find_last_of('.'));
        auto choose_tex = [&](const std::string& suffix)->std::string {
            std::string tga = tex_prefix + suffix + ".tga";
            std::string png = tex_prefix + suffix + ".png";
            if(std::filesystem::exists(tga)) return tga;
            if(std::filesystem::exists(png)) return png;
            return "";
        };
        if(mtl.features & Material::USE_DIFFUSE_MAP) 
            mtl.diffuse_tex_id = texMgr->load_texture(choose_tex("_diffuse"));
        if(mtl.features & Material::USE_NORMAL_MAP) 
            mtl.normal_tex_id = texMgr->load_texture(choose_tex("_nm"));
        if(mtl.features & Material::USE_SPECULAR_MAP) 
            mtl.specular_tex_id = texMgr->load_texture(choose_tex("_spec"));
        if(mtl.features & Material::USE_NM_TANGENT_MAP) 
            mtl.nm_tangent_tex_id = texMgr->load_texture(choose_tex("_nm_tangent"));

        /* 绑定材质 */
        int mtl_id = matMgr->add_material(mtl);
        mesh.material_id = mtl_id;
        mesh.name = obj_path.substr(obj_path.find_last_of('/') + 1, obj_path.find_last_of('.') - obj_path.find_last_of('/') - 1);                        
    }


    // 辅助函数：解析并添加模型与实例
    static void parse_models_and_entities(const json& cfg, Scene& scene, std::unique_ptr<ShaderManager>& shaderMgr,
                                      std::unique_ptr<TextureManager>& texMgr, std::unique_ptr<MaterialManager>& matMgr,
                                      std::unique_ptr<ModelManager>& modelMgr, std::unique_ptr<EntityManager>& entityMgr) {
        std::unordered_map<std::string, int> ref_to_id;

        /* 解析模型 */
        if(cfg.contains("models") && cfg["models"].is_object()) {
            for(auto& [model_key, m_info] : cfg["models"].items()) {
                int model_id = modelMgr->create_empty_model(); 
                ref_to_id[model_key] = model_id;
                
                std::string base_path = m_info.value("path", "");
                if(m_info.contains("mesh") && m_info["mesh"].is_object()) {
                    for(auto& [mesh_name, mesh_cfg] : m_info["mesh"].items()) {
                        load_single_mesh(mesh_cfg, base_path, model_id, modelMgr, matMgr, texMgr);
                    }
                }
                std::cout << "Model loaded: " << base_path << " (ID: " << model_id << ")" << std::endl;
            }
        } else {
            std::cerr << "Model configuration is missing." << std::endl;
            exit(-1);
        }

        /* 解析实例 */
        auto process_entity = [&](const std::string& name, const json& e_cfg) {
            std::string ref = e_cfg.value("ref", "");
            if(ref_to_id.find(ref) == ref_to_id.end()) {
                std::cerr << "Warning: Entity '" << name << "' refers to '" << ref << "' which was not found!" << std::endl;
                return;
            }
            int model_id = ref_to_id[ref];
            auto& entity = entityMgr->create_entity(model_id);
            
            entity.set_pos({e_cfg["pos"][0], e_cfg["pos"][1], e_cfg["pos"][2]})
            .set_rot({e_cfg["rot"][0], e_cfg["rot"][1], e_cfg["rot"][2]})
            .set_scale({e_cfg["scale"][0], e_cfg["scale"][1], e_cfg["scale"][2]});

            scene.add_entity(&entity);
        };

        if (cfg.contains("entity") && cfg["entity"].is_object()) {
            for (auto it = cfg["entity"].begin(); it != cfg["entity"].end(); ++it) {
                process_entity(it.key(), it.value());
            }
        }
    }

    // 辅助函数：解析并添加多个光源
    static void parse_lights(const json& cfg, Scene& scene) {
        auto parse_and_add_light = [&](const json& l_cfg) {
            Light light;
            light.position = {l_cfg["pos"][0], l_cfg["pos"][1], l_cfg["pos"][2]};
            light.intensity = {l_cfg["intensity"][0], l_cfg["intensity"][1], l_cfg["intensity"][2]};
            scene.add_light(light);
        };

        if(cfg.contains("light")) {
            if(cfg["light"].is_array()) {
                for(auto& l_cfg : cfg["light"]) parse_and_add_light(l_cfg);
            } else if(cfg["light"].is_object()) {
                parse_and_add_light(cfg["light"]);
            }
        } else {
            std::cerr << "Light configuration is missing." << std::endl;
            exit(-1);
        }
    }

    static int parse_features(const json& f_cfg) {
        int mask = 0;
        if(f_cfg.is_array()) {
            for(auto& f : f_cfg) {
                if(f == "USE_DIFFUSE_MAP")  mask |= Material::USE_DIFFUSE_MAP;
                if(f == "USE_NORMAL_MAP")   mask |= Material::USE_NORMAL_MAP;
                if(f == "USE_SPECULAR_MAP") mask |= Material::USE_SPECULAR_MAP;
                if(f == "USE_NM_TANGENT_MAP") mask |= Material::USE_NM_TANGENT_MAP;
            }
        }
        return mask;
    }
};
