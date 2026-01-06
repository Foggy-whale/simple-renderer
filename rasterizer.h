#pragma once
#include "geometry.h"
#include "triangle.h"
#include "model.h"
#include "scene.h"
#include "tgaimage.h"
#include "shader.h"

enum class Buffers {
    Color = 1,
    Depth = 2
};
inline Buffers operator|(Buffers a, Buffers b) {
    return Buffers((int)a | (int)b);
}
inline Buffers operator&(Buffers a, Buffers b) {
    return Buffers((int)a & (int)b);
}

class Rasterizer {
private:
    int width, height;
    int ssaa = 1;
    std::vector<vec3> framebuffer;
    std::vector<float> zbuffer;
    mat4 model, view, projection;
    IShader* currentShader;
    
    template<typename T> inline T get_avg(const int& ind, const std::vector<T>& buffer_data);

    void draw_line(vec2 v1, vec2 v2, TGAColor color);
    void draw_triangle(const Triangle& triangle);
    void draw_model(const Model& model);
    
public:
    Rasterizer() = default;
    Rasterizer(int width, int height) : width(width), height(height) {
        framebuffer.resize(width * height);
        zbuffer.resize(width * height);
    }
    ~Rasterizer() = default;

    TGAImage to_tga_image(Buffers buffer);
    void draw(const Scene& scene);
    void save_as(const std::string& filename);
    void save_zbuffer_as(const std::string& filename);
    void clear(Buffers buffer) {
        if((buffer & Buffers::Color) == Buffers::Color) std::fill(framebuffer.begin(), framebuffer.end(), vec3(0, 0, 0));
        if((buffer & Buffers::Depth) == Buffers::Depth) std::fill(zbuffer.begin(), zbuffer.end(), 0);
    }
    
    void set_model(const mat4& m) { model = m; }
    void set_view(const mat4& m) { view = m; }
    void set_projection(const mat4& m) { projection = m; }
    void set_pixel(const int& ind, const vec3& color) { framebuffer[ind] = color; }
    void set_depth(const int& ind, const float& z) { zbuffer[ind] = z; }
    
    float get_depth(const int& ind) const { return zbuffer[ind]; }
    vec3 get_pixel(const int& ind) const { return framebuffer[ind]; }
    std::vector<vec3>& get_framebuffer() { return framebuffer; }
    std::vector<float>& get_zbuffer() { return zbuffer; }
    
    void enable_ssaa(const int& ssaa) { 
        this->ssaa = ssaa;
        framebuffer.resize(width * height * ssaa * ssaa);
        zbuffer.resize(width * height * ssaa * ssaa);
    }
};