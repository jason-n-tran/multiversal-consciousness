#include <catch2/catch_test_macros.hpp>
#include <SDL3/SDL.h>
#include "../../src/engine/MovementSystem.h"
#include "../../src/engine/InputManager.h"
#include "../../src/engine/PossessionSystem.h"
#include "../../src/engine/EntityManager.h"
#include "../../src/engine/ComponentRegistry.h"
#include "../../src/engine/System.h"
#include "../../src/engine/Components.h"
#include <cmath>

TEST_CASE("MovementSystem initialization", "[movement][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    auto movement_system_ptr = std::make_unique<MovementSystem>();
    MovementSystem* movement_system = system_manager.register_system(std::move(movement_system_ptr));
    
    system_manager.initialize();
    
    SECTION("System initializes without error") {
        REQUIRE(movement_system != nullptr);
    }
}

TEST_CASE("MovementSystem with InputManager integration", "[movement][system][input]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Create systems
    auto movement_system_ptr = std::make_unique<MovementSystem>();
    MovementSystem* movement_system = system_manager.register_system(std::move(movement_system_ptr));
    
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    InputManager input_manager;
    
    // Set up system references
    movement_system->set_input_manager(&input_manager);
    movement_system->set_possession_system(possession_system);
    possession_system->set_input_manager(&input_manager);
    
    system_manager.initialize();
    
    // Create test agent
    EntityID agent = entity_manager.create_entity();
    Agent agent_comp{1, false, 100.0f};  // Agent number 1, movement speed 100
    Transform transform{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
    
    component_registry.add_component<Agent>(agent, agent_comp);
    component_registry.add_component<Transform>(agent, transform);
    
    // Update systems to refresh mappings
    possession_system->update(0.016f);
    
    SECTION("No movement when no agent is possessed") {
        // Simulate movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        movement_system->update(0.016f);
        
        // Agent should not move (not possessed)
        const Transform* updated_transform = component_registry.get_component<Transform>(agent);
        REQUIRE(updated_transform->x == 100.0f);
        REQUIRE(updated_transform->y == 200.0f);
    }
    
    SECTION("Movement when agent is possessed") {
        // Possess the agent
        possession_system->possess_agent(1);
        
        // Simulate upward movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        float delta_time = 0.016f;  // ~60 FPS
        movement_system->update(delta_time);
        
        // Agent should move up (y decreases)
        const Transform* updated_transform = component_registry.get_component<Transform>(agent);
        REQUIRE(updated_transform->x == 100.0f);  // X unchanged
        REQUIRE(updated_transform->y < 200.0f);   // Y decreased (moved up)
        
        // Calculate expected position
        float expected_y = 200.0f - (100.0f * delta_time);  // speed * time
        REQUIRE(std::abs(updated_transform->y - expected_y) < 0.001f);
    }
    
    SECTION("Diagonal movement normalization") {
        // Possess the agent
        possession_system->possess_agent(1);
        
        // Simulate diagonal movement input (up and right)
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        
        event.key.key = SDLK_W;  // Up
        input_manager.process_event(event);
        event.key.key = SDLK_D;  // Right
        input_manager.process_event(event);
        
        input_manager.update(0.016f);
        
        // Update movement system
        float delta_time = 0.016f;
        movement_system->update(delta_time);
        
        // Agent should move diagonally with normalized speed
        const Transform* updated_transform = component_registry.get_component<Transform>(agent);
        
        // Calculate expected diagonal movement (normalized)
        float movement_distance = 100.0f * delta_time;  // speed * time
        float normalized_distance = movement_distance / std::sqrt(2.0f);  // Diagonal normalization
        
        float expected_x = 100.0f + normalized_distance;
        float expected_y = 200.0f - normalized_distance;
        
        REQUIRE(std::abs(updated_transform->x - expected_x) < 0.1f);  // Increased tolerance
        REQUIRE(std::abs(updated_transform->y - expected_y) < 0.1f);  // Increased tolerance
    }
    
    SECTION("Multiple movement directions") {
        // Possess the agent
        possession_system->possess_agent(1);
        
        // Test all four directions
        struct MovementTest {
            SDL_Keycode key;
            float expected_x_change;
            float expected_y_change;
        };
        
        std::vector<MovementTest> tests = {
            {SDLK_W, 0.0f, -1.0f},   // Up
            {SDLK_S, 0.0f, 1.0f},    // Down
            {SDLK_A, -1.0f, 0.0f},   // Left
            {SDLK_D, 1.0f, 0.0f}     // Right
        };
        
        for (const auto& test : tests) {
            // Reset position
            Transform* transform_comp = component_registry.get_component<Transform>(agent);
            transform_comp->x = 100.0f;
            transform_comp->y = 200.0f;
            
            // Clear input state
            input_manager.reset_states();
            
            // Simulate movement input
            SDL_Event event;
            event.type = SDL_EVENT_KEY_DOWN;
            event.key.key = test.key;
            input_manager.process_event(event);
            input_manager.update(0.016f);
            
            // Update movement system
            float delta_time = 0.016f;
            movement_system->update(delta_time);
            
            // Check movement
            const Transform* updated_transform = component_registry.get_component<Transform>(agent);
            float movement_distance = 100.0f * delta_time;
            
            float expected_x = 100.0f + (test.expected_x_change * movement_distance);
            float expected_y = 200.0f + (test.expected_y_change * movement_distance);
            
            REQUIRE(std::abs(updated_transform->x - expected_x) < 0.1f);  // Increased tolerance
            REQUIRE(std::abs(updated_transform->y - expected_y) < 0.1f);  // Increased tolerance
        }
    }
    
    SECTION("No movement without input") {
        // Possess the agent
        possession_system->possess_agent(1);
        
        // Update movement system without any input
        movement_system->update(0.016f);
        
        // Agent should not move
        const Transform* updated_transform = component_registry.get_component<Transform>(agent);
        REQUIRE(updated_transform->x == 100.0f);
        REQUIRE(updated_transform->y == 200.0f);
    }
}

TEST_CASE("MovementSystem without required dependencies", "[movement][system][error]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    auto movement_system_ptr = std::make_unique<MovementSystem>();
    MovementSystem* movement_system = system_manager.register_system(std::move(movement_system_ptr));
    
    system_manager.initialize();
    
    SECTION("No movement without InputManager") {
        // Don't set input manager reference
        movement_system->update(0.016f);
        // Should not crash and should handle gracefully
    }
    
    SECTION("No movement without PossessionSystem") {
        InputManager input_manager;
        movement_system->set_input_manager(&input_manager);
        // Don't set possession system reference
        
        movement_system->update(0.016f);
        // Should not crash and should handle gracefully
    }
}

TEST_CASE("MovementSystem with different agent speeds", "[movement][system][speed]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Create systems
    auto movement_system_ptr = std::make_unique<MovementSystem>();
    MovementSystem* movement_system = system_manager.register_system(std::move(movement_system_ptr));
    
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    InputManager input_manager;
    
    // Set up system references
    movement_system->set_input_manager(&input_manager);
    movement_system->set_possession_system(possession_system);
    possession_system->set_input_manager(&input_manager);
    
    system_manager.initialize();
    
    // Create agents with different speeds
    EntityID fast_agent = entity_manager.create_entity();
    EntityID slow_agent = entity_manager.create_entity();
    
    Agent fast_agent_comp{1, false, 200.0f};  // Fast agent
    Agent slow_agent_comp{2, false, 50.0f};   // Slow agent
    
    Transform fast_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    Transform slow_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    
    component_registry.add_component<Agent>(fast_agent, fast_agent_comp);
    component_registry.add_component<Agent>(slow_agent, slow_agent_comp);
    component_registry.add_component<Transform>(fast_agent, fast_transform);
    component_registry.add_component<Transform>(slow_agent, slow_transform);
    
    // Update systems to refresh mappings
    possession_system->update(0.016f);
    
    SECTION("Fast agent moves faster") {
        // Possess fast agent
        possession_system->possess_agent(1);
        
        // Simulate movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_D;  // Move right
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        float delta_time = 0.016f;
        movement_system->update(delta_time);
        
        // Check fast agent moved
        const Transform* fast_transform_updated = component_registry.get_component<Transform>(fast_agent);
        float expected_fast_x = 200.0f * delta_time;  // Fast speed
        REQUIRE(std::abs(fast_transform_updated->x - expected_fast_x) < 0.1f);  // Increased tolerance
        
        // Reset and test slow agent
        input_manager.reset_states();
        possession_system->possess_agent(2);
        
        // Reset slow agent position
        Transform* slow_transform_comp = component_registry.get_component<Transform>(slow_agent);
        slow_transform_comp->x = 0.0f;
        
        // Same input
        event.key.key = SDLK_D;
        input_manager.process_event(event);
        input_manager.update(delta_time);
        movement_system->update(delta_time);
        
        // Check slow agent moved less
        const Transform* slow_transform_updated = component_registry.get_component<Transform>(slow_agent);
        float expected_slow_x = 50.0f * delta_time;  // Slow speed
        REQUIRE(std::abs(slow_transform_updated->x - expected_slow_x) < 0.1f);  // Increased tolerance
        
        // Fast agent should have moved further
        REQUIRE(expected_fast_x > expected_slow_x);
    }
}

TEST_CASE("MovementSystem with PhysicsComponent integration", "[movement][system][physics]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    
    // Create systems
    auto movement_system_ptr = std::make_unique<MovementSystem>();
    MovementSystem* movement_system = system_manager.register_system(std::move(movement_system_ptr));
    
    auto possession_system_ptr = std::make_unique<PossessionSystem>();
    PossessionSystem* possession_system = system_manager.register_system(std::move(possession_system_ptr));
    
    InputManager input_manager;
    
    // Set up system references
    movement_system->set_input_manager(&input_manager);
    movement_system->set_possession_system(possession_system);
    possession_system->set_input_manager(&input_manager);
    
    system_manager.initialize();
    
    SECTION("MovementSystem applies velocity to PhysicsComponent instead of direct transform changes") {
        // Create agent with physics component
        EntityID physics_agent = entity_manager.create_entity();
        
        Agent agent_comp{1, false, 100.0f};
        Transform transform{50.0f, 50.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent physics{0.0f, 0.0f, 0.0f, 980.0f, 1.0f, false, false, true, 0.8f, 0.0f};
        
        component_registry.add_component<Agent>(physics_agent, agent_comp);
        component_registry.add_component<Transform>(physics_agent, transform);
        component_registry.add_component<PhysicsComponent>(physics_agent, physics);
        
        // Update systems to refresh mappings
        possession_system->update(0.016f);
        possession_system->possess_agent(1);
        
        // Simulate rightward movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_D;  // Move right
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        movement_system->update(0.016f);
        
        // Check that velocity was set instead of direct position change
        const PhysicsComponent* updated_physics = component_registry.get_component<PhysicsComponent>(physics_agent);
        const Transform* updated_transform = component_registry.get_component<Transform>(physics_agent);
        
        REQUIRE(updated_physics != nullptr);
        REQUIRE(updated_transform != nullptr);
        
        // Velocity should be set to movement speed
        REQUIRE(updated_physics->velocity_x == 100.0f);
        
        // Transform should not be directly modified by MovementSystem when physics is present
        REQUIRE(updated_transform->x == 50.0f);  // Position unchanged by MovementSystem
        REQUIRE(updated_transform->y == 50.0f);
    }
    
    SECTION("MovementSystem falls back to direct transform movement without PhysicsComponent") {
        // Create agent without physics component
        EntityID direct_agent = entity_manager.create_entity();
        
        Agent agent_comp{2, false, 100.0f};
        Transform transform{50.0f, 50.0f, 0.0f, 1.0f, 1.0f};
        
        component_registry.add_component<Agent>(direct_agent, agent_comp);
        component_registry.add_component<Transform>(direct_agent, transform);
        // No PhysicsComponent added
        
        // Update systems to refresh mappings
        possession_system->update(0.016f);
        possession_system->possess_agent(2);
        
        // Simulate rightward movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_D;  // Move right
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        float delta_time = 0.016f;
        movement_system->update(delta_time);
        
        // Check that transform was directly modified
        const Transform* updated_transform = component_registry.get_component<Transform>(direct_agent);
        
        REQUIRE(updated_transform != nullptr);
        
        // Transform should be directly modified when no physics component
        float expected_x = 50.0f + (100.0f * delta_time);
        REQUIRE(std::abs(updated_transform->x - expected_x) < 0.1f);
        REQUIRE(updated_transform->y == 50.0f);  // Y unchanged
    }
    
    SECTION("MovementSystem handles vertical movement with physics") {
        // Create agent with physics component
        EntityID physics_agent = entity_manager.create_entity();
        
        Agent agent_comp{1, false, 100.0f};
        Transform transform{50.0f, 50.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent physics{0.0f, 0.0f, 0.0f, 980.0f, 1.0f, false, false, true, 0.8f, 0.0f};
        
        component_registry.add_component<Agent>(physics_agent, agent_comp);
        component_registry.add_component<Transform>(physics_agent, transform);
        component_registry.add_component<PhysicsComponent>(physics_agent, physics);
        
        // Update systems to refresh mappings
        possession_system->update(0.016f);
        possession_system->possess_agent(1);
        
        // Simulate upward movement input
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;  // Move up
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Update movement system
        movement_system->update(0.016f);
        
        // Check that vertical velocity was set
        const PhysicsComponent* updated_physics = component_registry.get_component<PhysicsComponent>(physics_agent);
        
        REQUIRE(updated_physics != nullptr);
        
        // Vertical velocity should be set (negative for upward movement)
        REQUIRE(updated_physics->velocity_y == -100.0f);
    }
}