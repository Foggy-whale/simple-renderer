#include <random>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int x0, int y0, int x1, int y1, TGAImage &framebuffer, TGAColor color);

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    for (int i = 0; i < (1 << 24); i++) {
        int x0 = rng() % width, y0 = rng() % height;
        int x1 = rng() % width, y1 = rng() % height;
        line(x0, y0, x1, y1, framebuffer, {static_cast<uint8_t>(rng() % 256), 
        static_cast<uint8_t>(rng() % 256), 
        static_cast<uint8_t>(rng() % 256), 
        static_cast<uint8_t>(rng() % 256)});
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
