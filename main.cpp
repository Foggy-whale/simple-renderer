#include <cmath>
#include <tuple>
#include "rasterizer.h"
#include "tgaimage.h"

constexpr int width  = 128;
constexpr int height = 128;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv) {
    Rasterizer r(width, height);

    r.triangle(vec2(  7, 45), vec2(35, 100), vec2(45,  60), red);
    r.triangle(vec2(120, 35), vec2(90,   5), vec2(45, 110), white);
    r.triangle(vec2(115, 83), vec2(80,  90), vec2(85, 120), green);

    r.save_as("framebuffer.tga");
    return 0;
}