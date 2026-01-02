#pragma once
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

class Rasterizer {
private:
    int width, height;
    TGAImage framebuffer;
public:
    Rasterizer() = default;
    Rasterizer(int width, int height) : width(width), height(height) {
        framebuffer = TGAImage(width, height, TGAImage::RGB);
    }
    TGAImage get_framebuffer() { return framebuffer; }
    void save_as(const std::string& filename) { framebuffer.write_tga_file(filename); }
    void line(vec2 v1, vec2 v2, TGAColor color);
    void triangle(vec2 v1, vec2 v2, vec2 v3, TGAColor color);
};