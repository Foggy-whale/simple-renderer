#include <cmath>
#include <iostream>
#include "rasterizer.h"
#include "tgaimage.h"
#include "random.h"
#include "scene.h"

constexpr int width  = 800;
constexpr int height = 800;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }
    vec3 eye_pos = 3 * vec3(std::cos(90 * M_PI / 180), 0, std::sin(90 * M_PI / 180));
    Scene scene;
    Model model(argv[1]);
    Camera camera;
    model.set_pos({0, 0, 0}).set_rot({0, 0, 0}).set_scale({1, 1, 1});
    scene.addModel(model);
    camera.set_eye(eye_pos).set_target({0, 0, 0}).set_up({0, 1, 0}).set_projection(45, width / height, 0.1, 100);
    scene.setCamera(camera);

    Rasterizer r(width, height);
    r.enable_ssaa(3);
    r.draw(scene);

    r.save_as("framebuffer.tga");
    r.save_zbuffer_as("zbuffer.tga");

    return 0;
}
