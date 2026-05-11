#include <catch2/catch_all.hpp>
#include "engine/PossessionSystem.h"
#include "engine/AgentRenderer.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/Components.h"
#include <SDL3/SDL.h>
#include <cmath>

TEST_CASE("PossessionSystem basic functionality", "[possession][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Register the possession system
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    // Initialize the system manager
    system_manager.initialize();
    
    SECTION("Initial state") {
        REQUIRE_FALSE(possession_system->get_possessed_entity().has_value());
        REQUIRE(possession_system->get_agent_mappings().empty());
    }
    
    SECTION("Agent creation and mapping") {
        // Create test agents
        EntityID agent1 = entity_manager.create_entity();
        EntityID agent2 = entity_manager.create_entity();
        EntityID agent3 = entity_manager.create_entity();
        
        // Add Agent components with different numbers
        Agent agent_comp1{1, false, 100.0f};
        Agent agent_comp2{2, false, 150.0f};
        Agent agent_comp3{5, false, 200.0f};
        
        component_registry.add_component<Agent>(agent1, agent_comp1);
        component_registry.add_component<Agent>(agent2, agent_comp2);
        component_registry.add_component<Agent>(agent3, agent_comp3);
        
        // Add Transform components (required for camera following)
        Transform transform1{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
        Transform transform2{300.0f, 400.0f, 0.0f, 1.0f, 1.0f};
        Transform transform3{500.0f, 600.0f, 0.0f, 1.0f, 1.0f};
        
        component_registry.add_component<Transform>(agent1, transform1);
        component_registry.add_component<Transform>(agent2, transform2);
        component_registry.add_component<Transform>(agent3, transform3);
        
        // Update system to refresh agent mappings
        possession_system->update(0.016f);
        
        const auto& mappings = possession_system->get_agent_mappings();
        REQUIRE(mappings.size() == 3);
        REQUIRE(mappings.at(1) == agent1);
        REQUIRE(mappings.at(2) == agent2);
        REQUIRE(mappings.at(5) == agent3);
    }
}

TEST_CASE("PossessionSystem agent possession", "[possession][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Register the possession system
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    // Initialize the system manager
    system_manager.initialize();
    
    // Create test agents
    EntityID agent1 = entity_manager.create_entity();
    EntityID agent2 = entity_manager.create_entity();
    
    Agent agent_comp1{1, false, 100.0f};
    Agent agent_comp2{2, false, 150.0f};
    
    component_registry.add_component<Agent>(agent1, agent_comp1);
    component_registry.add_component<Agent>(agent2, agent_comp2);
    
    Transform transform1{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
    Transform transform2{300.0f, 400.0f, 0.0f, 1.0f, 1.0f};
    
    component_registry.add_component<Transform>(agent1, transform1);
    component_registry.add_component<Transform>(agent2, transform2);
    
    // Update to refresh mappings
    possession_system->update(0.016f);
    
    SECTION("Possess agent by number") {
        REQUIRE(possession_system->possess_agent(1));
        
        auto possessed = possession_system->get_possessed_entity();
        REQUIRE(possessed.has_value());
        REQUIRE(possessed.value() == agent1);
        
        // Check agent component state
        const Agent* agent = component_registry.get_component<Agent>(agent1);
        REQUIRE(agent != nullptr);
        REQUIRE(agent->is_possessed);
        
        // Check other agent is not possessed
        const Agent* other_agent = component_registry.get_component<Agent>(agent2);
        REQUIRE(other_agent != nullptr);
        REQUIRE_FALSE(other_agent->is_possessed);
        
        REQUIRE(possession_system->is_entity_possessed(agent1));
        REQUIRE_FALSE(possession_system->is_entity_possessed(agent2));
    }
    
    SECTION("Switch possession between agents") {
        // Possess first agent
        REQUIRE(possession_system->possess_agent(1));
        REQUIRE(possession_system->get_possessed_entity().value() == agent1);
        
        const Agent* agent1_comp = component_registry.get_component<Agent>(agent1);
        REQUIRE(agent1_comp->is_possessed);
        
        // Switch to second agent
        REQUIRE(possession_system->possess_agent(2));
        REQUIRE(possession_system->get_possessed_entity().value() == agent2);
        
        // Check states after switch
        agent1_comp = component_registry.get_component<Agent>(agent1);
        const Agent* agent2_comp = component_registry.get_component<Agent>(agent2);
        
        REQUIRE_FALSE(agent1_comp->is_possessed);  // Should be released
        REQUIRE(agent2_comp->is_possessed);        // Should be possessed
    }
    
    SECTION("Release possession") {
        // Possess an agent first
        REQUIRE(possession_system->possess_agent(1));
        REQUIRE(possession_system->get_possessed_entity().has_value());
        
        // Release possession
        possession_system->release_possession();
        REQUIRE_FALSE(possession_system->get_possessed_entity().has_value());
        
        // Check agent state
        const Agent* agent = component_registry.get_component<Agent>(agent1);
        REQUIRE_FALSE(agent->is_possessed);
    }
    
    SECTION("Invalid agent possession") {
        REQUIRE_FALSE(possession_system->possess_agent(0));   // Invalid number (too low)
        REQUIRE_FALSE(possession_system->possess_agent(10));  // Invalid number (too high)
        REQUIRE_FALSE(possession_system->possess_agent(3));   // Agent doesn't exist
        
        REQUIRE_FALSE(possession_system->get_possessed_entity().has_value());
    }
}

TEST_CASE("PossessionSystem agent number retrieval", "[possession][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Register the possession system
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    // Initialize the system manager
    system_manager.initialize();
    
    // Create test agents
    EntityID agent1 = entity_manager.create_entity();
    EntityID agent2 = entity_manager.create_entity();
    EntityID non_agent = entity_manager.create_entity();
    
    Agent agent_comp1{3, false, 100.0f};
    Agent agent_comp2{7, false, 150.0f};
    
    component_registry.add_component<Agent>(agent1, agent_comp1);
    component_registry.add_component<Agent>(agent2, agent_comp2);
    // non_agent has no Agent component
    
    SECTION("Get agent numbers") {
        REQUIRE(possession_system->get_agent_number(agent1) == 3);
        REQUIRE(possession_system->get_agent_number(agent2) == 7);
        REQUIRE(possession_system->get_agent_number(non_agent) == 0);  // Not an agent
    }
}

TEST_CASE("PossessionSystem input handling", "[possession][system][input]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Register the possession system
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    // Initialize the system manager
    system_manager.initialize();
    
    // Create test agents
    EntityID agent1 = entity_manager.create_entity();
    EntityID agent2 = entity_manager.create_entity();
    
    Agent agent_comp1{1, false, 100.0f};
    Agent agent_comp2{2, false, 150.0f};
    
    component_registry.add_component<Agent>(agent1, agent_comp1);
    component_registry.add_component<Agent>(agent2, agent_comp2);
    
    Transform transform1{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
    Transform transform2{300.0f, 400.0f, 0.0f, 1.0f, 1.0f};
    
    component_registry.add_component<Transform>(agent1, transform1);
    component_registry.add_component<Transform>(agent2, transform2);
    
    // Update to refresh mappings
    possession_system->update(0.016f);
    
    SECTION("Handle number key input") {
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        
        // Test key 1
        event.key.key = SDLK_1;
        REQUIRE(possession_system->handle_input(event));
        REQUIRE(possession_system->get_possessed_entity().value() == agent1);
        
        // Test key 2
        event.key.key = SDLK_2;
        REQUIRE(possession_system->handle_input(event));
        REQUIRE(possession_system->get_possessed_entity().value() == agent2);
        
        // Test invalid key
        event.key.key = SDLK_3;  // No agent with number 3
        REQUIRE_FALSE(possession_system->handle_input(event));
        REQUIRE(possession_system->get_possessed_entity().value() == agent2);  // Should remain unchanged
        
        // Test non-number key
        event.key.key = SDLK_A;
        REQUIRE_FALSE(possession_system->handle_input(event));
    }
    
    SECTION("Handle non-key-down events") {
        SDL_Event event;
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = SDLK_1;
        
        REQUIRE_FALSE(possession_system->handle_input(event));
        REQUIRE_FALSE(possession_system->get_possessed_entity().has_value());
    }
}

TEST_CASE("CameraController basic functionality", "[camera][possession]") {
    ComponentRegistry component_registry;
    CameraController camera;
    
    camera.initialize(component_registry, 800, 600);
    
    SECTION("Initial state") {
        REQUIRE(camera.get_x() == 0.0f);
        REQUIRE(camera.get_y() == 0.0f);
        REQUIRE_FALSE(camera.get_target_entity().has_value());
    }
    
    SECTION("Set position") {
        camera.set_position(100.0f, 200.0f);
        REQUIRE(camera.get_x() == 100.0f);
        REQUIRE(camera.get_y() == 200.0f);
    }
    
    SECTION("Camera bounds") {
        CameraBounds bounds{-500.0f, 500.0f, -300.0f, 300.0f};
        camera.set_bounds(bounds);
        
        const auto& retrieved_bounds = camera.get_bounds();
        REQUIRE(retrieved_bounds.min_x == -500.0f);
        REQUIRE(retrieved_bounds.max_x == 500.0f);
        REQUIRE(retrieved_bounds.min_y == -300.0f);
        REQUIRE(retrieved_bounds.max_y == 300.0f);
    }
    
    SECTION("Follow speed and smoothing") {
        camera.set_follow_speed(5.0f);
        REQUIRE(camera.get_follow_speed() == 5.0f);
        
        camera.set_smoothing_factor(0.8f);
        REQUIRE(camera.get_smoothing_factor() == 0.8f);
        
        // Test clamping
        camera.set_smoothing_factor(1.5f);  // Should be clamped to 1.0
        REQUIRE(camera.get_smoothing_factor() == 1.0f);
        
        camera.set_smoothing_factor(-0.5f);  // Should be clamped to 0.0
        REQUIRE(camera.get_smoothing_factor() == 0.0f);
    }
    
    SECTION("Coordinate conversion") {
        camera.set_position(100.0f, 200.0f);
        
        int screen_x, screen_y;
        camera.world_to_screen(150.0f, 250.0f, screen_x, screen_y);
        
        // World (150, 250) with camera at (100, 200) should be at screen center + offset
        REQUIRE(screen_x == 450);  // (150 - 100) + (800 / 2)
        REQUIRE(screen_y == 350);  // (250 - 200) + (600 / 2)
        
        float world_x, world_y;
        camera.screen_to_world(screen_x, screen_y, world_x, world_y);
        REQUIRE(world_x == Catch::Approx(150.0f));
        REQUIRE(world_y == Catch::Approx(250.0f));
    }
}

TEST_CASE("CameraController entity following", "[camera][possession]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    CameraController camera;
    
    camera.initialize(component_registry, 800, 600);
    
    // Create test entity with transform
    EntityID entity = entity_manager.create_entity();
    Transform transform{300.0f, 400.0f, 0.0f, 1.0f, 1.0f};
    component_registry.add_component<Transform>(entity, transform);
    
    SECTION("Set target entity") {
        camera.set_target_entity(entity);
        REQUIRE(camera.get_target_entity().has_value());
        REQUIRE(camera.get_target_entity().value() == entity);
        
        // Update should move camera towards entity
        camera.update(0.016f);
        
        // Camera should move towards the entity position
        // (exact position depends on smoothing, but should be closer)
        REQUIRE(camera.get_x() != 0.0f);
        REQUIRE(camera.get_y() != 0.0f);
    }
    
    SECTION("Stop following") {
        camera.set_target_entity(entity);
        camera.set_smoothing_factor(0.0f);  // Disable smoothing for predictable behavior
        camera.update(0.016f);
        
        camera.set_target_entity(std::nullopt);
        REQUIRE_FALSE(camera.get_target_entity().has_value());
        
        // Camera position should not change when not following
        float x_before = camera.get_x();
        float y_before = camera.get_y();
        
        camera.update(0.016f);
        
        // Since we're not following anything and smoothing is disabled, position should remain exactly the same
        REQUIRE(camera.get_x() == x_before);
        REQUIRE(camera.get_y() == y_before);
    }
}

TEST_CASE("AgentRenderer visual configuration", "[agent][renderer][visual]") {
    AgentRenderer renderer;
    
    SECTION("Default visual configuration") {
        const auto& config = renderer.get_visual_config();
        
        // Check default values are reasonable
        REQUIRE(config.possessed_outline_width > 0.0f);
        REQUIRE(config.possessed_glow_radius > 0.0f);
        REQUIRE(config.idle_outline_width > 0.0f);
        REQUIRE(config.glow_pulse_speed > 0.0f);
        REQUIRE(config.glow_pulse_min >= 0.0f);
        REQUIRE(config.glow_pulse_max <= 1.0f);
        REQUIRE(config.glow_pulse_min < config.glow_pulse_max);
    }
    
    SECTION("Set visual configuration") {
        AgentVisualConfig new_config;
        new_config.possessed_outline_width = 3.0f;
        new_config.possessed_glow_radius = 12.0f;
        new_config.glow_pulse_speed = 1.5f;
        
        renderer.set_visual_config(new_config);
        
        const auto& config = renderer.get_visual_config();
        REQUIRE(config.possessed_outline_width == 3.0f);
        REQUIRE(config.possessed_glow_radius == 12.0f);
        REQUIRE(config.glow_pulse_speed == 1.5f);
    }
}

TEST_CASE("PossessionSystem integration with camera and renderer", "[possession][integration]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Register systems
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    auto agent_renderer_ptr = std::make_unique<AgentRenderer>();
    AgentRenderer* agent_renderer = system_manager.register_system(std::move(agent_renderer_ptr));
    
    // Initialize systems
    system_manager.initialize();
    
    // Set up integration
    possession_system->set_agent_renderer(agent_renderer);
    
    // Create test agent
    EntityID agent = entity_manager.create_entity();
    Agent agent_comp{1, false, 100.0f};
    Transform transform{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
    
    component_registry.add_component<Agent>(agent, agent_comp);
    component_registry.add_component<Transform>(agent, transform);
    
    // Update to refresh mappings
    possession_system->update(0.016f);
    
    SECTION("Camera follows possessed agent") {
        // Possess the agent
        REQUIRE(possession_system->possess_agent(1));
        
        // Get camera controller
        const auto& camera = possession_system->get_camera_controller();
        REQUIRE(camera.get_target_entity().has_value());
        REQUIRE(camera.get_target_entity().value() == agent);
        
        // Update camera
        possession_system->update(0.016f);
        
        // Camera should move towards agent position
        REQUIRE(camera.get_x() != 0.0f);
        REQUIRE(camera.get_y() != 0.0f);
    }
    
    SECTION("Camera stops following when possession is released") {
        // Possess then release
        possession_system->possess_agent(1);
        possession_system->release_possession();
        
        const auto& camera = possession_system->get_camera_controller();
        REQUIRE_FALSE(camera.get_target_entity().has_value());
    }
    
    SECTION("Camera bounds can be set") {
        CameraBounds bounds{-1000.0f, 1000.0f, -500.0f, 500.0f};
        possession_system->set_camera_bounds(bounds);
        
        const auto& camera = possession_system->get_camera_controller();
        const auto& retrieved_bounds = camera.get_bounds();
        REQUIRE(retrieved_bounds.min_x == -1000.0f);
        REQUIRE(retrieved_bounds.max_x == 1000.0f);
    }
}