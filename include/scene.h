#pragma once
#include <vector>
#include <memory>
#include "model.h"
#include "camera.h"
#include "shader.h"

class Scene {
private:
    Camera activeCamera;
    std::vector<Model> models;
    std::vector<Light> lights;
public:
    void add_model(Model m) { models.push_back(m); }
    void add_light(Light l) { lights.push_back(l); }
    void set_camera(const Camera& c) { activeCamera = c; }
    
    Camera& get_camera() { return activeCamera; }
    const Camera& get_camera() const { return activeCamera; }
    const std::vector<Model>& get_models() const { return models; }
    const std::vector<Light>& get_lights() const { return lights; }
};
