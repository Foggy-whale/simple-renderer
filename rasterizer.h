#pragma once
#include "geometry.h"
#include "triangle.h"
#include "model.h"
#include "scene.h"
#include "tgaimage.h"
#include "shader.h"

class Rasterizer {
private:
    int width, height;
    int ssaa = 1;
    std::vector<vec3> framebuffer;
    std::vector<float> zbuffer;
    mat4 model, view, projection;
    void draw_line(vec2 v1, vec2 v2, TGAColor color);
    void draw_triangle(const Triangle& triangle, IShader& shader);
    void draw_model(const Model& model, IShader& shader);
public:
    Rasterizer() = default;
    Rasterizer(int width, int height) : width(width), height(height) {
        framebuffer.resize(width * height);
        zbuffer.resize(width * height);
    }
    std::vector<vec3>& get_framebuffer() { return framebuffer; }
    std::vector<float>& get_zbuffer() { return zbuffer; }
    void save_as(const std::string& filename) { 
        int sample_factor = ssaa * ssaa;
        TGAImage img(width, height, TGAImage::RGB);
        for(int i = 0; i < width * height; i++) {
            vec3 color(0, 0, 0);
            for(int j = 0; j < sample_factor; j++) {
                color += framebuffer[i * sample_factor + j];
            }
            color /= sample_factor;
            img.set(i % width, i / width, {static_cast<uint8_t>(color.z * 255), 
                                           static_cast<uint8_t>(color.y * 255), 
                                           static_cast<uint8_t>(color.x * 255), 
                                           255});
        }
        img.write_tga_file(filename); 
    }
    void save_zbuffer_as(const std::string& filename);
    
    void set_model(const mat4& m) { model = m; }
    void set_view(const mat4& m) { view = m; }
    void set_projection(const mat4& m) { projection = m; }

    float get_depth(const int& ind) const { return zbuffer[ind]; }
    vec3 get_pixel(const int& ind) const { return framebuffer[ind]; }
    void set_pixel(const int& ind, const vec3& color) { framebuffer[ind] = color; }
    void set_depth(const int& ind, const float& z) { zbuffer[ind] = z; }
    void enable_ssaa(const int& ssaa) { 
        this->ssaa = ssaa;
        framebuffer.resize(width * height * ssaa * ssaa);
        zbuffer.resize(width * height * ssaa * ssaa);
    }
    
    void clear() {
        std::fill(framebuffer.begin(), framebuffer.end(), vec3(0, 0, 0));
        std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::max());
    }
    
    TGAImage to_tga_image() {
        int sample_factor = ssaa * ssaa;
        TGAImage img(width, height, TGAImage::RGB);
        for(int i = 0; i < width * height; i++) {
            vec3 color(0, 0, 0);
            for(int j = 0; j < sample_factor; j++) {
                color += framebuffer[i * sample_factor + j];
            }
            color /= sample_factor;
            img.set(i % width, i / width, {static_cast<uint8_t>(color.z * 255), 
                                           static_cast<uint8_t>(color.y * 255), 
                                           static_cast<uint8_t>(color.x * 255), 
                                           255});
        }
        return img;
    }

    void draw(const Scene& scene);
};