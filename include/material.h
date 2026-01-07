#pragma once
#include "geometry.h"
#include "tgaimage.h"

struct MaterialParameters {
    vec3 diffuse_color = {1.f, 1.f, 1.f};
    vec3 ambient = {0.05f, 0.05f, 0.05f};
    vec3 diffuse = {0.2f, 0.3f, 0.4f};
    vec3 specular = {0.7937f, 0.7937f, 0.7937f};
    float shininess = 150.f;
};

class IShader;
class Material {
private:
    MaterialParameters params;
    TGAImage* diffuse_map = nullptr; 
    TGAImage* normal_map  = nullptr;
    IShader* shader = nullptr;
public:
    Material() = default;
    Material(const MaterialParameters& params) : params(params) {}
    
    Material& set_params(const MaterialParameters& params) { this->params = params; return *this; }
    Material& set_diffuse_map(TGAImage* img) { diffuse_map = img; return *this; }
    Material& set_normal_map(TGAImage* img) { normal_map = img; return *this; }
    Material& set_shader(IShader* s) { shader = s; return *this; }
    
    const MaterialParameters& get_params() const { return params; }
    MaterialParameters& get_params() { return params; }
    TGAImage* get_diffuse_map() const { return diffuse_map; }
    TGAImage* get_normal_map() const { return normal_map; }
    IShader* get_shader() const { return shader; }
};