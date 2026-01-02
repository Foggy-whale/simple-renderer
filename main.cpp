#include <cmath>
#include <tuple>
#include "rasterizer.h"
#include "tgaimage.h"
#include "random.h"

constexpr int width  = 800;
constexpr int height = 800;

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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Rasterizer r(width, height);
    Model model(argv[1]);
    Random rng;

    for(int i = 0; i < model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        TGAColor rnd;
        for(int c = 0; c < 3; c++) rnd[c] = rng(256);
        r.triangle({ax, ay}, {bx, by}, {cx, cy}, rnd);
    }
    r.save_as("framebuffer.tga");

    return 0;
}