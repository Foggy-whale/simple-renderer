#pragma once
#include <vector>
#include "model.h"
#include "camera.h"

#include <memory>
#include "shader.h"

class Scene {
private:
    std::vector<Model> models;
    Camera mainCamera;
    std::vector<Light> lights;
    std::shared_ptr<IShader> shader;
public:
    void addModel(Model m) { models.push_back(m); }
    void setCamera(const Camera& c) { mainCamera = c; }
    void addLight(Light l) { lights.push_back(l); }
    void setShader(std::shared_ptr<IShader> s) { shader = s; }
    
    const std::vector<Model>& getModels() const { return models; }
    const Camera& getCamera() const { return mainCamera; }
    const std::vector<Light>& getLights() const { return lights; }
    std::shared_ptr<IShader> getShader() const { return shader; }
};
