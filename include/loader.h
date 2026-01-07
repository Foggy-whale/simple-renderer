#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include "global.h"
#include "scene.h"
#include "rasterizer.h"
#include "shader.h"

using json = nlohmann::json;

class Loader {
public:
    static bool load(const std::string& config_path, const std::string& scene_name, 
                     Scene& scene, ShaderManager& shaderManager) {
        std::ifstream in(config_path);
        if(in.fail()) {
            std::cerr << "Cannot open config file: " << config_path << std::endl;
            return false;
        }

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

        auto& cfg = data[scene_name];

        // --- 解析 Camera ---
        if(cfg.contains("camera")) {
            auto& c_cfg = cfg["camera"];
            Camera camera;
            camera.set_eye({c_cfg["eye"][0], c_cfg["eye"][1], c_cfg["eye"][2]})
                  .set_target({c_cfg["target"][0], c_cfg["target"][1], c_cfg["target"][2]})
                  .set_up({c_cfg["up"][0], c_cfg["up"][1], c_cfg["up"][2]})
                  .set_projection(45.0f, (float)width / height, 0.1f, 100.0f); 
            scene.set_camera(camera);
        }

        // --- 解析 Models ---
        if(cfg.contains("model")) {
            if(cfg["model"].is_array()) {
                for(auto& m_cfg : cfg["model"]) parse_and_add_model(m_cfg, scene, shaderManager);
            } else if(cfg["model"].is_object()) {
                parse_and_add_model(cfg["model"], scene, shaderManager);
            }
        }

        // --- 解析 Lights ---
        if(cfg.contains("light")) {
            if(cfg["light"].is_array()) {
                for(auto& l_cfg : cfg["light"]) parse_and_add_light(l_cfg, scene);
            } else if(cfg["light"].is_object()) {
                parse_and_add_light(cfg["light"], scene);
            }
        }

        return true;
    }

private:
    // 辅助函数：解析并添加单个模型
    static void parse_and_add_model(const json& m_cfg, Scene& scene, ShaderManager& shaderManager) {
        std::string obj_path = m_cfg["path"];

        Model model(obj_path);
        Material material;
        MaterialParameters params;
        std::string shader_name = "flat"; 

        if(m_cfg.contains("material")) {
            auto& mat_cfg = m_cfg["material"];
            shader_name = mat_cfg.value("shader", "flat");
            
            params.diffuse_color = {mat_cfg["diffuse_color"][0], mat_cfg["diffuse_color"][1], mat_cfg["diffuse_color"][2]};
            params.ambient       = {mat_cfg["ambient"][0], mat_cfg["ambient"][1], mat_cfg["ambient"][2]};
            params.diffuse       = {mat_cfg["diffuse"][0], mat_cfg["diffuse"][1], mat_cfg["diffuse"][2]};
            params.specular      = {mat_cfg["specular"][0], mat_cfg["specular"][1], mat_cfg["specular"][2]};
            params.shininess     = mat_cfg.value("shininess", 150.0f);
        }

        auto s_ptr = shaderManager.get_shader(shader_name);
        if (!s_ptr) s_ptr = shaderManager.get_shader("flat");
        
        material.set_shader(s_ptr);
        material.set_params(params);

        model.set_pos({m_cfg["pos"][0], m_cfg["pos"][1], m_cfg["pos"][2]})
             .set_rot({m_cfg["rot"][0], m_cfg["rot"][1], m_cfg["rot"][2]})
             .set_scale({m_cfg["scale"][0], m_cfg["scale"][1], m_cfg["scale"][2]})
             .set_material(material);
        
        scene.add_model(model);
    }

    // 辅助函数：解析并添加单个光源
    static void parse_and_add_light(const json& l_cfg, Scene& scene) {
        Light light = {
            {l_cfg["pos"][0], l_cfg["pos"][1], l_cfg["pos"][2]},
            {l_cfg["intensity"][0], l_cfg["intensity"][1], l_cfg["intensity"][2]}
        };
        scene.add_light(light);
    }
};