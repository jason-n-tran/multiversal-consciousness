#include <iostream>
#include "engine/GameEngine.h"
#include "engine/TileRenderer.h"

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
    config.tile_size = 32;
    config.render_scale = 1.0f;
    
    // Initialize engine
    if (!engine.initialize(config)) {
        std::cerr << "Failed to initialize game engine" << std::endl;
        return -1;
    }

    auto tile_renderer = std::make_unique<TileRenderer>(config);
    TileRenderer* renderer_ptr = engine.register_system(std::move(tile_renderer));
    
    auto tile_map = std::make_unique<TileMap>();
    tile_map->initialize(20, 15);
    
    renderer_ptr->create_solid_texture(1, {0.2f, 0.8f, 0.2f, 1.0f}, 32, 32, engine.get_renderer());
    renderer_ptr->create_solid_texture(2, {0.8f, 0.2f, 0.2f, 1.0f}, 32, 32, engine.get_renderer());
    renderer_ptr->create_solid_texture(3, {0.2f, 0.2f, 0.8f, 1.0f}, 32, 32, engine.get_renderer());
    renderer_ptr->create_solid_texture(4, {0.8f, 0.8f, 0.2f, 1.0f}, 32, 32, engine.get_renderer());
    
    for (int y = 0; y < tile_map->height; ++y) {
        for (int x = 0; x < tile_map->width; ++x) {
            Tile tile;
            
            if ((x + y) % 4 == 0) {
                tile.texture_id = 1;
            } else if ((x + y) % 4 == 1) {
                tile.texture_id = 2;
            } else if ((x + y) % 4 == 2) {
                tile.texture_id = 3;
            } else {
                tile.texture_id = 4;
            }
            
            if (x == 0 || x == tile_map->width - 1 || y == 0 || y == tile_map->height - 1) {
                tile.texture_id = 0;
                tile.color = {0.5f, 0.5f, 0.5f, 1.0f};
            }
            
            tile_map->set_tile(x, y, tile);
        }
    }
    
    renderer_ptr->set_tile_map(std::move(tile_map));
    
    renderer_ptr->set_camera_position(320.0f, 240.0f);
    renderer_ptr->set_show_grid(true);
    
    std::cout << "Engine initialized successfully!" << std::endl;
    std::cout << "Tile renderer demo loaded with 20x15 tile map" << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;
    
    // Run the engine
    engine.run();
    
    // Engine will automatically shutdown via RAII destructor
    std::cout << "Application terminated successfully" << std::endl;
    return 0;
}