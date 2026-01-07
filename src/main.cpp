#include <cmath>
#include "tgaimage.h"
#include "scene.h"
#include "rasterizer.h"
#include "shader.h"
#include "loader.h"
#include "display.h"


void parse_command(int argc, char** argv, std::string& scene_name, Modes& mode);

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " scene_name [-r|-v]" << std::endl;
        return -1;
    }

    std::string scene_name;
    Modes mode = Modes::NORMAL;

    // Parse command line arguments
    parse_command(argc, argv, scene_name, mode);
    if(scene_name.empty()) {
        std::cerr << "Error: No scene name specified." << std::endl;
        return -1;
    }

    // Initialize Shader Manager 
    auto shaderManager = std::make_unique<ShaderManager>();
    shaderManager->register_shader("flat", std::make_unique<FlatShader>());
    shaderManager->register_shader("phong", std::make_unique<PhongShader>());
    shaderManager->register_shader("gouraud", std::make_unique<GouraudShader>());
    
    // Initialize Scene and Rasterizer
    Scene scene;
    Rasterizer r(width, height);

    std::cout << "--- Initializing Scene: " << scene_name << " ---" << std::endl;
    try {
        if(!Loader::load(config_path, scene_name, scene, *shaderManager)) {
            std::cerr << "Failed to load scene: " << scene_name << std::endl;
            return -1;
        }
    } catch(const std::exception& e) {
        std::cerr << "Critical Error during loading: " << e.what() << std::endl;
        return -1;
    }
    
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

void parse_command(int argc, char** argv, std::string& scene_name, Modes& mode) {
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
            }
        }
        else scene_name = arg;
    }
}

