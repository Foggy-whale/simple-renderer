#pragma once
#include <vector>
#include <memory>
#include "model.h"
#include "light.h"
#include "camera.h"
#include "shader.h"

class Scene {
private:
    Camera activeCamera;
    std::vector<Light> lights;
    std::vector<Entity*> entities;
public:
    void set_camera(const Camera& c) { activeCamera = c; }
    void add_light(const Light& l) { lights.push_back(std::move(l)); }
    void add_entity(Entity* e) { entities.push_back(std::move(e)); }
    
    Camera& get_camera() { return activeCamera; }
    const Camera& get_camera() const { return activeCamera; }
    const std::vector<Light>& get_lights() const { return lights; }
    const std::vector<Entity*>& get_entities() const { return entities; }
};
