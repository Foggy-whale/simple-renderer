#include <cmath>
#include <tuple>
#include "rasterizer.h"
#include "tgaimage.h"
#include "random.h"

constexpr int width  = 64;
constexpr int height = 64;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

std::pair<float, float> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width / 2.f,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height / 2.f }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

int main(int argc, char** argv) {
    Rasterizer r(width, height);

    float ax = 17, ay =  4, az =  13;
    float bx = 55, by = 39, bz = 128;
    float cx = 23, cy = 59, cz = 255;

    r.triangle({ax, ay, az}, {bx, by, bz}, {cx, cy, cz});
    r.save_as("framebuffer.tga");

    return 0;
}