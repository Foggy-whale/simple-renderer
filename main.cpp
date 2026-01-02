#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor gray    = {200, 200, 200, 255}; 
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int x0, int y0, int x1, int y1, TGAImage &framebuffer, TGAColor color);

std::tuple<int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width / 2,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height / 2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i = 0; i < model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    for (int i = 0; i < model.nverts(); i++) { // iterate through all vertices
        vec3 v = model.vert(i);            // get i-th vertex
        auto [x, y] = project(v);          // project it to the screen
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage &framebuffer, TGAColor color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        framebuffer.set(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
