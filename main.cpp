#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.h"
#include "tgaimage.h"
#include "scene.h"
#include "shader.h"

enum class Modes {
    NORMAL,
    ROTATE,
    VISUAL
};

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

void process_command(int argc, char** argv, Modes& mode, std::string& obj_file, int& shader_type);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj [-r|-v|-g|-p]" << std::endl;
        return 1;
    }
    
    Modes mode = Modes::NORMAL;
    std::string obj_file;
    int shader_type = 0; 

    process_command(argc, argv, mode, obj_file, shader_type);
    if(obj_file.empty()) {
        std::cerr << "Error: No OBJ file specified." << std::endl;
        return -1;
    }
    
    // Initialize Scene and Rasterizer
    Scene scene;
    Rasterizer r(width, height);

    // Initialize Shader Manager 
    auto shaderManager = std::make_unique<ShaderManager>();
    shaderManager->register_shader("flat", std::make_unique<FlatShader>());
    shaderManager->register_shader("phong", std::make_unique<PhongShader>());
    shaderManager->register_shader("gouraud", std::make_unique<GouraudShader>());
    
    // Load Model 
    Model model(obj_file);
    Material material;
    switch(shader_type) {
        case 0:
            material.set_shader(shaderManager->get_shader("flat"));
            break;
        case 1:
            material.set_shader(shaderManager->get_shader("gouraud"));
            break;
        case 2:
            material.set_shader(shaderManager->get_shader("phong"));
            break;
    }
    model.set_pos({0, 0, 0}).set_rot({0, 0, 0}).set_scale({0.1, 0.1, 0.1}).set_material(material);
    scene.add_model(model);
    
    // Initialize Camera 
    Camera camera;
    vec3 eye = {-1, 4, 6};
    vec3 target = {0, 2, 0};
    vec3 up = {0, 1, 0};
    camera.set_eye(eye).set_target(target).set_up(up).set_projection(45, (float)width / height, 0.1, 100);
    scene.set_camera(camera);
    
    // Add lights 
    Light l1 = {{4, 8, 4}, {80, 80, 80}};
    // Light l2 = {{-4, 4, 0}, {50, 50, 50}};
    scene.add_light(l1);
    // scene.add_light(l2);
    
    // Run selected mode 
    std::unique_ptr<IRenderMode> renderMode;
    switch(mode) {
        case Modes::NORMAL:
            renderMode = std::make_unique<NormalMode>();
            break;
        case Modes::ROTATE:
            renderMode = std::make_unique<RotateMode>();
            break;
        case Modes::VISUAL:
            renderMode = std::make_unique<VisualMode>();
            break;
    }
    renderMode->run(r, scene);

    return 0; 
}

void process_command(int argc, char** argv, Modes& mode, std::string& obj_file, int& shader_type) {
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg[0] == '-') {
            char op = arg[1];
            switch(op) {
                case 'v': 
                    mode = Modes::VISUAL;
                    break;
                case 'r':
                    mode = Modes::ROTATE;
                    break;
                case 'g':
                    shader_type = 1;
                    break;
                case 'p':
                    shader_type = 2;
                    break;
            }
        }
        else obj_file = arg;
    }
}

void NormalMode::run(Rasterizer& r, Scene& scene) {
    r.enable_ssaa(3); 
    try {
        r.draw(scene); 
    } catch(const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
    r.save_as("framebuffer.tga"); 
    r.save_zbuffer_as("zbuffer.tga"); 
    std::cout << "Rendered to framebuffer.tga" << std::endl; 
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