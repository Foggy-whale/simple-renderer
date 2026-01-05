#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.h"
#include "tgaimage.h"
#include "scene.h"
#include "shader.h"


constexpr int width  = 1600;
constexpr int height = 1600;


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj [-r|-v]" << std::endl;
        return 1;
    }
    
    std::string mode = "";
    std::string obj_file;
    int shader_type = 0; // Only Flat 
    
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg == "-v" || arg == "-r") {
            mode = arg;
        } else {
            obj_file = arg;
        }
    }
    
    if(obj_file.empty()) {
        std::cerr << "Error: No OBJ file specified." << std::endl;
        return 1;
    }
    
    Scene scene;
    Model model(obj_file);
    Camera camera;
    model.set_pos({0, 0, 0}).set_rot({0, 0, 0}).set_scale({1, 1, 1});
    scene.addModel(model);
    
    // Set shader: always Flat 
    std::cout << "Using Flat Shader" << std::endl;
    scene.setShader(std::make_shared<FlatShader>());
    
    // Initial Camera 
    vec3 eye = {-2, 0, 2};
    vec3 target = {0, 0, 0};
    vec3 up = {0, 1, 0};
    camera.set_eye(eye).set_target(target).set_up(up).set_projection(45, (float)width / height, 0.1, 100);
    
    // Add lights 
    Light l1 = {{4, 4, 4}, {50, 50, 50}};
    Light l2 = {{-4, 4, 0}, {50, 50, 50}};
    scene.addLight(l1);
    scene.addLight(l2);


    Rasterizer r(width, height);
    
    if (mode == "") { 
        // Normal mode: render single frame and save to file 
        scene.setCamera(camera); 
        r.enable_ssaa(3); 
        r.draw(scene); 
        
        r.save_as("framebuffer.tga"); 
        r.save_zbuffer_as("zbuffer.tga"); 
        std::cout << "Rendered to framebuffer.tga" << std::endl; 
        
    } else if (mode == "-r") { 
        // Rotate mode 
        r.enable_ssaa(1); 
        float angle = 0.0f; 
        float radius = 3.0f; 
        
        std::cout << "Starting auto-rotation... Press ESC to exit." << std::endl; 
        
        while(true) { 
            float x = radius * std::sin(angle); 
            float z = radius * std::cos(angle); 
            camera.set_eye({x, 0, z}).set_target({0,0,0}); 
            scene.setCamera(camera); 
            
            r.clear(); 
            r.draw(scene); 
            
            TGAImage tga = r.to_tga_image(); 
            cv::Mat image(height, width, CV_8UC3, tga.buffer()); 
            cv::flip(image, image, 0); 
            
            cv::imshow("TinyRenderer - Auto Rotate", image); 
            int key = cv::waitKey(10); 
            if (key == 27) break; 
            
            angle += 0.03f; 
        } 
    } else if (mode == "-v") { 
        // Interactive mode 
        r.enable_ssaa(2); 
        
        // Camera state 
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
            target = eye + front.normalized(); 
            camera.set_eye(eye).set_target(target); 
            scene.setCamera(camera); 
        }; 
        
        // Init direction based on initial eye/target 
        vec3 front = (target - eye).normalized(); 
        pitch = asin(front.y) * 180.0f / M_PI; 
        yaw = atan2(front.z, front.x) * 180.0f / M_PI; 
        
        std::cout << "Starting interactive mode..." << std::endl; 
        std::cout << "Controls: WASD (Move), JK (Up/Down), Arrows (Look), ESC (Exit)" << std::endl; 
        
        // OpenCV waitKey is blocking for delay milliseconds. 
        // It returns only ONE key pressed during that interval. 
        // This makes simultaneous movement (e.g. W+A) or Move+Look impossible with simple waitKey loop. 
        // However, standard OpenCV highgui doesn't support polling multiple keys or key state. 
        // BUT, we can use a small hack: small delay, and rely on key repeat? No, repeat is slow. 
        // Or we can assume 'shift' or other modifiers? No. 
        // For true simultaneous input we need a different library (SDL/GLFW) or OS calls. 
        // BUT, since user asked to "fix" it within this context (OpenCV), we can try to improve it. 
        // Actually, we can't fix "simultaneous" key presses with cv::waitKey as it returns the last key pressed in the queue or just one. 
        // Wait, does it? 
        // If we reduce waitKey delay to 1, we can loop faster. 
        // But we still only get one key per frame. 
        // Unless we process ALL pending keys per frame? 
        // cv::waitKey(1) returns -1 if no key. 
        // We can try to drain the event queue? 
        // while((key = cv::waitKey(1)) != -1) { ... handle key ... } 
        // This might allow processing multiple buffered key presses? 
        // Let's try that. 
        
        while(true) { 
            update_camera(); 
            
            r.clear(); 
            r.draw(scene); 
            
            TGAImage tga = r.to_tga_image(); 
            cv::Mat image(height, width, CV_8UC3, tga.buffer()); 
            cv::flip(image, image, 0); 
            
            cv::imshow("TinyRenderer - Interactive", image); 
            
            // Handle all pending keys 
            int key = -1; 
            // We use a small delay for the first call to wait and show image 
            // Then check if other keys are queued? 
            // Actually waitKey(1) waits 1ms. 
            // If we loop waitKey(1) until -1, we consume time. 
            // Let's just process one key per fast loop? 
            // Or try to process multiple. 
            
            key = cv::waitKey(1); 
            if (key == 27) break; 
            
            // Movement vectors 
            vec3 f = (target - eye).normalized(); 
            vec3 r_vec = cross_product(f, up).normalized(); // Right vector 
            vec3 u = up; 
            
            // To support "simultaneous" feeling, we need key state. 
            // OpenCV doesn't give key state. 
            // The user might be pressing 'w' and 'a'. The OS sends 'w' 'w' 'w' ... or 'w' 'a' 'w' 'a'. 
            // If we only process one, it feels jerky. 
            // Since we are limited to OpenCV, we can't do true async input. 
            // BUT, we can increase speed/sensitivity so single key presses do more. 
            // And maybe process a few keys if they are in buffer? 
            // Let's try a loop to drain buffer. 
            
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


    return 0; 
}
