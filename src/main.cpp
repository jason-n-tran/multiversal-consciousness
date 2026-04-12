#include <iostream>
#include "engine/GameEngine.h"

int main(int argc, char* argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    std::cout << "Quantum Bifurcation Game Engine" << std::endl;
    std::cout << "================================" << std::endl;
    
    // Create engine instance
    GameEngine engine;
    
    // Configure engine
    EngineConfig config;
    config.window_title = "Quantum Bifurcation - Foundation Test";
    config.window_width = 1280;
    config.window_height = 720;
    config.fullscreen = false;
    config.vsync = true;
    
    // Initialize engine
    if (!engine.initialize(config)) {
        std::cerr << "Failed to initialize game engine" << std::endl;
        return -1;
    }
    
    std::cout << "Engine initialized successfully!" << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;
    
    // Run the engine
    engine.run();
    
    // Engine will automatically shutdown via RAII destructor
    std::cout << "Application terminated successfully" << std::endl;
    return 0;
}