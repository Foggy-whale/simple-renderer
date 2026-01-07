#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.h"
#include "scene.h"

enum class Modes {
    NORMAL,
    ROTATE,
    VISUAL
};

static std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

class IRenderMode {
public:
    virtual ~IRenderMode() = default;
    virtual void run(Rasterizer& r, Scene& scene) = 0;
};

class NormalMode : public IRenderMode {
public:
    void run(Rasterizer& r, Scene& scene) override;
};

class RotateMode : public IRenderMode {
public:
    void run(Rasterizer& r, Scene& scene) override;
};

class VisualMode : public IRenderMode {
public:
    void run(Rasterizer& r, Scene& scene) override;
};

void NormalMode::run(Rasterizer& r, Scene& scene) {
    r.enable_ssaa(3); 

    std::cout << "--- Rendering Start :) ---" << std::endl;
    std::string timestamp = get_current_timestamp() + ".tga";
    try {
        r.draw(scene); 
    } catch(const std::runtime_error& e) {
        std::cout << "--- Rendering Failed! :< ---" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
    r.save_as("output/framebuffer_" + timestamp); 
    r.save_zbuffer_as("output/zbuffer_" + timestamp); 
    std::cout << "--- Rendering Completed! :> ---" << std::endl;
}

void RotateMode::run(Rasterizer& r, Scene& scene) {
    r.enable_ssaa(1); 

    Camera& camera = scene.get_camera();
    float angle = 0.0f; 
    float radius = 6.1f; 

    std::cout << "Starting auto-rotation... Press ESC to exit." << std::endl; 

    while(true) { 
        float x = radius * std::sin(angle); 
        float z = radius * std::cos(angle); 
        camera.set_eye({x, 4, z}).set_target({0, 2, 0}); 
        scene.set_camera(camera); 
    
        r.clear(Buffers::Color | Buffers::Depth); 
        try {
            r.draw(scene); 
        } catch(const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            exit(-1);
        }

        TGAImage tga = r.to_tga_image(Buffers::Color); 
        cv::Mat image(height, width, CV_8UC3, tga.buffer()); 
        cv::flip(image, image, 0); 
        cv::imshow("TinyRenderer - Auto Rotate", image); 

        int key = cv::waitKey(10);
        static bool paused = false;
        if(key == ' ') paused = !paused;
        if(key == 27) break; 
        if(paused) continue; 
        angle += 0.03f; 
    } 
}

void VisualMode::run(Rasterizer& r, Scene& scene) {
    r.enable_ssaa(1); 

    // Camera state 
    Camera& camera = scene.get_camera();
    vec3 eye = camera.get_eye();
    vec3 target = camera.get_target();
    vec3 up = camera.get_up();
    float yaw = -90.0f; 
    float pitch = 0.0f; 
    float speed = 0.3f; // Increased movement speed 
    float sensitivity = 3.0f; // Increased rotation sensitivity
    
    // Recalculate vectors 
    auto update_camera = [&]() { 
        vec3 front; 
        front.x = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f); 
        front.y = sin(pitch * M_PI / 180.0f); 
        front.z = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f); 
        vec3 target = eye + front.normalized(); 
        camera.set_target(target); 
    }; 
    
    // Init direction based on initial eye/target 
    vec3 front = (target - eye).normalized(); 
    pitch = asin(front.y) * 180.0f / M_PI; 
    yaw = atan2(front.z, front.x) * 180.0f / M_PI; 

    std::cout << "Starting interactive mode..." << std::endl; 
    std::cout << "Controls: WASD (Move), JK (Up/Down), Arrows (Look), ESC (Exit)" << std::endl; 

    while(true) { 
        update_camera(); 
        
        r.clear(Buffers::Color | Buffers::Depth); 
        try {
            r.draw(scene); 
        } catch(const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            exit(-1);
        }

        TGAImage tga = r.to_tga_image(Buffers::Color); 
        cv::Mat image(height, width, CV_8UC3, tga.buffer()); 
        cv::flip(image, image, 0); 
        
        cv::imshow("TinyRenderer - Interactive", image); 
        int key = -1; 
        key = cv::waitKey(1); 
        if (key == 27) break; 
        
        // Movement vectors 
        vec3 f = (target - eye).normalized(); 
        vec3 r_vec = cross_product(f, up).normalized(); // Right vector 
        vec3 u = up; 
        
        while(key != -1) { 
            if (key == 27) goto end_loop; 
            
            // Movement 
            if (key == 'w') eye = eye + f * speed; 
            if (key == 's') eye = eye - f * speed; 
            if (key == 'a') eye = eye - r_vec * speed; 
            if (key == 'd') eye = eye + r_vec * speed; 
            if (key == 'j') eye = eye + u * speed; // Up 
            if (key == 'k') eye = eye - u * speed; // Down 
            
            if (key == 0 || key == 63232) pitch += sensitivity; // Up 
            if (key == 1 || key == 63233) pitch -= sensitivity; // Down 
            if (key == 2 || key == 63234) yaw -= sensitivity;   // Left 
            if (key == 3 || key == 63235) yaw += sensitivity;   // Right 
            
            key = cv::waitKey(1); // Check next key immediately 
        } 
        
        // Constrain pitch 
        if (pitch > 89.0f) pitch = 89.0f; 
        if (pitch < -89.0f) pitch = -89.0f; 
    } 
    end_loop:;
}
