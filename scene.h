#pragma once
#include <vector>
#include "model.h"
#include "camera.h"

class Scene {
private:
    std::vector<Model> models;
    Camera mainCamera;
    // vec3 lightDir; 
public:
    void addModel(Model m) { models.push_back(m); }
    void setCamera(const Camera& c) { mainCamera = c; }
    const std::vector<Model>& getModels() const { return models; }
    const Camera& getCamera() const { return mainCamera; }
};
