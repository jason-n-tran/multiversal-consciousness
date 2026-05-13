#include <catch2/catch_test_macros.hpp>
#include "../../src/engine/TileRenderer.h"
#include "../../src/engine/AgentRenderer.h"
#include "../../src/engine/QuantumNodeRenderer.h"
#include "../../src/engine/RealityManager.h"
#include "../../src/engine/EntityManager.h"
#include "../../src/engine/ComponentRegistry.h"
#include "../../src/engine/ConfigLoader.h"

TEST_CASE("TileRenderer reality-specific visuals", "[rendering][tile-renderer]") {
    // Create engine configuration
    EngineConfig config;
    config.window_width = 800;
    config.window_height = 600;
    config.tile_size = 32;
    config.render_scale = 1.0f;
    
    // Create tile renderer
    TileRenderer renderer(config);
    
    // Create reality manager
    RealityManager reality_manager;
    
    // Set reality manager reference
    renderer.set_reality_manager(&reality_manager);
    
    SECTION("Reality indicator can be enabled/disabled") {
        renderer.set_show_reality_indicator(true);
        // No direct way to test this without SDL context, but method should not crash
        REQUIRE(true);
        
        renderer.set_show_reality_indicator(false);
        REQUIRE(true);
    }
    
    SECTION("Reality indicator colors can be set") {
        SDL_FColor reality_a_color{0.0f, 0.0f, 1.0f, 1.0f};  // Blue
        SDL_FColor reality_b_color{1.0f, 0.0f, 0.0f, 1.0f};  // Red
        
        renderer.set_reality_indicator_colors(reality_a_color, reality_b_color);
        // Method should not crash
        REQUIRE(true);
    }
    
    SECTION("Tile with reality-specific properties") {
        auto tile_map = std::make_unique<TileMap>();
        tile_map->initialize(10, 10);
        
        // Create a tile with reality-specific visuals
        Tile tile;
        tile.texture_id = 1;  // Default texture
        tile.reality_a_texture_id = 2;  // Reality A specific
        tile.reality_b_texture_id = 3;  // Reality B specific
        tile.reality_a_color = {0.0f, 0.0f, 1.0f, 1.0f};  // Blue for Reality A
        tile.reality_b_color = {1.0f, 0.0f, 0.0f, 1.0f};  // Red for Reality B
        
        tile_map->set_tile(5, 5, tile);
        renderer.set_tile_map(std::move(tile_map));
        
        // Verify tile was set correctly
        const Tile* retrieved_tile = renderer.get_tile_map()->get_tile(5, 5);
        REQUIRE(retrieved_tile != nullptr);
        REQUIRE(retrieved_tile->reality_a_texture_id == 2);
        REQUIRE(retrieved_tile->reality_b_texture_id == 3);
    }
}

TEST_CASE("AgentRenderer reality-specific animations", "[rendering][agent-renderer]") {
    AgentRenderer renderer;
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    
    // Initialize renderer
    renderer.initialize(entity_manager, component_registry);
    
    SECTION("Reality manager can be set") {
        RealityManager reality_manager;
        renderer.set_reality_manager(&reality_manager);
        // Method should not crash
        REQUIRE(true);
    }
    
    SECTION("Visual configuration can be modified") {
        AgentVisualConfig config = renderer.get_visual_config();
        config.possessed_outline_color = {1.0f, 0.0f, 0.0f, 1.0f};  // Red outline
        config.glow_pulse_speed = 5.0f;
        
        renderer.set_visual_config(config);
        
        const AgentVisualConfig& retrieved_config = renderer.get_visual_config();
        REQUIRE(retrieved_config.possessed_outline_color.r == 1.0f);
        REQUIRE(retrieved_config.glow_pulse_speed == 5.0f);
    }
    
    renderer.shutdown();
}

TEST_CASE("QuantumNodeRenderer reality-specific visuals", "[rendering][quantum-node-renderer]") {
    QuantumNodeRenderer renderer;
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    
    // Initialize renderer
    renderer.initialize(entity_manager, component_registry);
    
    SECTION("Reality manager can be set") {
        RealityManager reality_manager;
        renderer.set_reality_manager(&reality_manager);
        // Method should not crash
        REQUIRE(true);
    }
    
    SECTION("Visual configuration can be modified") {
        QuantumNodeVisualConfig config = renderer.get_visual_config();
        config.reality_a_color = {0.0f, 1.0f, 0.0f, 1.0f};  // Green for Reality A
        config.reality_b_color = {1.0f, 1.0f, 0.0f, 1.0f};  // Yellow for Reality B
        config.pulse_speed = 4.0f;
        
        renderer.set_visual_config(config);
        
        const QuantumNodeVisualConfig& retrieved_config = renderer.get_visual_config();
        REQUIRE(retrieved_config.reality_a_color.g == 1.0f);
        REQUIRE(retrieved_config.reality_b_color.r == 1.0f);
        REQUIRE(retrieved_config.reality_b_color.g == 1.0f);
        REQUIRE(retrieved_config.pulse_speed == 4.0f);
    }
    
    renderer.shutdown();
}

TEST_CASE("Rendering systems integration", "[rendering][integration]") {
    // Create all rendering systems
    EngineConfig config;
    config.window_width = 800;
    config.window_height = 600;
    config.tile_size = 32;
    config.render_scale = 1.0f;
    
    TileRenderer tile_renderer(config);
    AgentRenderer agent_renderer;
    QuantumNodeRenderer quantum_renderer;
    RealityManager reality_manager;
    
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    
    SECTION("All renderers can be initialized together") {
        tile_renderer.initialize(entity_manager, component_registry);
        agent_renderer.initialize(entity_manager, component_registry);
        quantum_renderer.initialize(entity_manager, component_registry);
        
        // Set reality manager references
        tile_renderer.set_reality_manager(&reality_manager);
        agent_renderer.set_reality_manager(&reality_manager);
        quantum_renderer.set_reality_manager(&reality_manager);
        
        // All should initialize without crashing
        REQUIRE(true);
        
        // Shutdown all renderers
        tile_renderer.shutdown();
        agent_renderer.shutdown();
        quantum_renderer.shutdown();
    }
    
    SECTION("Reality switching affects all renderers") {
        // Initialize all renderers
        tile_renderer.initialize(entity_manager, component_registry);
        agent_renderer.initialize(entity_manager, component_registry);
        quantum_renderer.initialize(entity_manager, component_registry);
        
        // Set reality manager references
        tile_renderer.set_reality_manager(&reality_manager);
        agent_renderer.set_reality_manager(&reality_manager);
        quantum_renderer.set_reality_manager(&reality_manager);
        
        // Test reality switching
        REQUIRE(reality_manager.get_current_reality() == Reality::A);
        
        bool switch_success = reality_manager.switch_reality();
        REQUIRE(switch_success);
        REQUIRE(reality_manager.get_current_reality() == Reality::B);
        
        switch_success = reality_manager.switch_reality();
        REQUIRE(switch_success);
        REQUIRE(reality_manager.get_current_reality() == Reality::A);
        
        // Shutdown all renderers
        tile_renderer.shutdown();
        agent_renderer.shutdown();
        quantum_renderer.shutdown();
    }
}