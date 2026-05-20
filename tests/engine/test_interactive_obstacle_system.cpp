#include <catch2/catch_test_macros.hpp>
#include "engine/InteractiveObstacleSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/InputManager.h"
#include "engine/PossessionSystem.h"
#include "engine/Components.h"

TEST_CASE("InteractiveObstacleSystem - Basic Functionality", "[interactive][obstacles]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    InputManager input_manager;
    PossessionSystem possession_system;
    InteractiveObstacleSystem obstacle_system;
    
    // Initialize systems
    possession_system.initialize(entity_manager, component_registry);
    obstacle_system.initialize(entity_manager, component_registry);
    obstacle_system.set_input_manager(&input_manager);
    obstacle_system.set_possession_system(&possession_system);
    
    SECTION("System initializes correctly") {
        REQUIRE(obstacle_system.get_nearby_interactable() == INVALID_ENTITY);
    }
    
    SECTION("Register and unregister obstacles") {
        // Create a tree obstacle
        EntityID tree_entity = entity_manager.create_entity();
        Transform tree_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        InteractableComponent tree_interactable;
        tree_interactable.type = InteractionType::Tree;
        tree_interactable.required_ability = AbilityType::Axe;
        tree_interactable.interaction_radius = 48.0f;
        
        component_registry.add_component<Transform>(tree_entity, tree_transform);
        component_registry.add_component<InteractableComponent>(tree_entity, tree_interactable);
        
        // Register the obstacle
        obstacle_system.register_obstacle(tree_entity);
        
        // Unregister the obstacle
        obstacle_system.unregister_obstacle(tree_entity);
        
        REQUIRE(obstacle_system.get_nearby_interactable() == INVALID_ENTITY);
    }
}

TEST_CASE("InteractiveObstacleSystem - Tree Obstacle Interaction", "[interactive][tree]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    InputManager input_manager;
    PossessionSystem possession_system;
    InteractiveObstacleSystem obstacle_system;
    
    // Initialize systems
    possession_system.initialize(entity_manager, component_registry);
    obstacle_system.initialize(entity_manager, component_registry);
    obstacle_system.set_input_manager(&input_manager);
    obstacle_system.set_possession_system(&possession_system);
    
    SECTION("Agent with Axe can interact with Tree") {
        // Create agent with Axe ability
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        loadout.current_ability = AbilityType::Axe;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create tree obstacle
        EntityID tree_entity = entity_manager.create_entity();
        Transform tree_transform{105.0f, 105.0f, 0.0f, 1.0f, 1.0f}; // Close to agent
        InteractableComponent tree_interactable;
        tree_interactable.type = InteractionType::Tree;
        tree_interactable.required_ability = AbilityType::Axe;
        tree_interactable.interaction_radius = 48.0f;
        
        component_registry.add_component<Transform>(tree_entity, tree_transform);
        component_registry.add_component<InteractableComponent>(tree_entity, tree_interactable);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(tree_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should be near the tree
        REQUIRE(obstacle_system.get_nearby_interactable() == tree_entity);
        
        // Force interaction should succeed
        bool interaction_success = obstacle_system.force_interaction(agent, tree_entity);
        REQUIRE(interaction_success == true);
        
        // Tree should be destroyed after interaction
        REQUIRE_FALSE(entity_manager.is_valid(tree_entity));
    }
    
    SECTION("Agent without Axe cannot interact with Tree") {
        // Create agent without Axe ability
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        loadout.current_ability = AbilityType::None;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create tree obstacle
        EntityID tree_entity = entity_manager.create_entity();
        Transform tree_transform{105.0f, 105.0f, 0.0f, 1.0f, 1.0f};
        InteractableComponent tree_interactable;
        tree_interactable.type = InteractionType::Tree;
        tree_interactable.required_ability = AbilityType::Axe;
        tree_interactable.interaction_radius = 48.0f;
        
        component_registry.add_component<Transform>(tree_entity, tree_transform);
        component_registry.add_component<InteractableComponent>(tree_entity, tree_interactable);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(tree_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should be near the tree
        REQUIRE(obstacle_system.get_nearby_interactable() == tree_entity);
        
        // Force interaction should fail
        bool interaction_success = obstacle_system.force_interaction(agent, tree_entity);
        REQUIRE(interaction_success == false);
        
        // Tree should still exist
        REQUIRE(entity_manager.is_valid(tree_entity));
    }
}

TEST_CASE("InteractiveObstacleSystem - Door Obstacle Interaction", "[interactive][door]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    InputManager input_manager;
    PossessionSystem possession_system;
    InteractiveObstacleSystem obstacle_system;
    
    // Initialize systems
    possession_system.initialize(entity_manager, component_registry);
    obstacle_system.initialize(entity_manager, component_registry);
    obstacle_system.set_input_manager(&input_manager);
    obstacle_system.set_possession_system(&possession_system);
    
    SECTION("Agent with Keycard can unlock Door") {
        // Create agent with Keycard ability
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        loadout.current_ability = AbilityType::Keycard;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create door obstacle
        EntityID door_entity = entity_manager.create_entity();
        Transform door_transform{105.0f, 105.0f, 0.0f, 1.0f, 1.0f};
        InteractableComponent door_interactable;
        door_interactable.type = InteractionType::Door;
        door_interactable.required_ability = AbilityType::Keycard;
        door_interactable.interaction_radius = 48.0f;
        
        Door door_component;
        door_component.is_locked = true;
        door_component.is_open = false;
        
        component_registry.add_component<Transform>(door_entity, door_transform);
        component_registry.add_component<InteractableComponent>(door_entity, door_interactable);
        component_registry.add_component<Door>(door_entity, door_component);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(door_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should be near the door
        REQUIRE(obstacle_system.get_nearby_interactable() == door_entity);
        
        // Force interaction should succeed
        bool interaction_success = obstacle_system.force_interaction(agent, door_entity);
        REQUIRE(interaction_success == true);
        
        // Door should be unlocked and opened
        const auto* updated_door = component_registry.get_component<Door>(door_entity);
        REQUIRE(updated_door != nullptr);
        REQUIRE(updated_door->is_locked == false);
        REQUIRE(updated_door->is_open == true);
        REQUIRE(updated_door->animation_progress == 1.0f);
        
        // Interactable should be marked as inactive
        const auto* updated_interactable = component_registry.get_component<InteractableComponent>(door_entity);
        REQUIRE(updated_interactable != nullptr);
        REQUIRE(updated_interactable->is_active == false);
    }
}

TEST_CASE("InteractiveObstacleSystem - Chasm Obstacle Interaction", "[interactive][chasm]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    InputManager input_manager;
    PossessionSystem possession_system;
    InteractiveObstacleSystem obstacle_system;
    
    // Initialize systems
    possession_system.initialize(entity_manager, component_registry);
    obstacle_system.initialize(entity_manager, component_registry);
    obstacle_system.set_input_manager(&input_manager);
    obstacle_system.set_possession_system(&possession_system);
    
    SECTION("Agent with DoubleJump can cross Chasm") {
        // Create agent with DoubleJump ability
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        loadout.current_ability = AbilityType::DoubleJump;
        
        PhysicsComponent physics;
        physics.velocity_y = 0.0f;
        physics.is_grounded = true;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        component_registry.add_component<PhysicsComponent>(agent, physics);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create chasm obstacle
        EntityID chasm_entity = entity_manager.create_entity();
        Transform chasm_transform{105.0f, 105.0f, 0.0f, 1.0f, 1.0f};
        InteractableComponent chasm_interactable;
        chasm_interactable.type = InteractionType::Chasm;
        chasm_interactable.required_ability = AbilityType::DoubleJump;
        chasm_interactable.interaction_radius = 64.0f;
        
        component_registry.add_component<Transform>(chasm_entity, chasm_transform);
        component_registry.add_component<InteractableComponent>(chasm_entity, chasm_interactable);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(chasm_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should be near the chasm
        REQUIRE(obstacle_system.get_nearby_interactable() == chasm_entity);
        
        // Force interaction should succeed
        bool interaction_success = obstacle_system.force_interaction(agent, chasm_entity);
        REQUIRE(interaction_success == true);
        
        // Agent should have upward velocity and not be grounded
        const auto* updated_physics = component_registry.get_component<PhysicsComponent>(agent);
        REQUIRE(updated_physics != nullptr);
        REQUIRE(updated_physics->velocity_y == -400.0f);
        REQUIRE(updated_physics->is_grounded == false);
    }
}

TEST_CASE("InteractiveObstacleSystem - Proximity Detection", "[interactive][proximity]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    InputManager input_manager;
    PossessionSystem possession_system;
    InteractiveObstacleSystem obstacle_system;
    
    // Initialize systems
    possession_system.initialize(entity_manager, component_registry);
    obstacle_system.initialize(entity_manager, component_registry);
    obstacle_system.set_input_manager(&input_manager);
    obstacle_system.set_possession_system(&possession_system);
    
    SECTION("Agent detects nearby obstacles within range") {
        // Create agent
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create obstacle within range
        EntityID obstacle_entity = entity_manager.create_entity();
        Transform obstacle_transform{120.0f, 120.0f, 0.0f, 1.0f, 1.0f}; // ~28 pixels away
        InteractableComponent obstacle_interactable;
        obstacle_interactable.type = InteractionType::Tree;
        obstacle_interactable.interaction_radius = 48.0f; // Should be in range
        
        component_registry.add_component<Transform>(obstacle_entity, obstacle_transform);
        component_registry.add_component<InteractableComponent>(obstacle_entity, obstacle_interactable);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(obstacle_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should detect the nearby obstacle
        REQUIRE(obstacle_system.get_nearby_interactable() == obstacle_entity);
    }
    
    SECTION("Agent does not detect obstacles outside range") {
        // Create agent
        EntityID agent = entity_manager.create_entity();
        Transform agent_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        LoadoutComponent loadout;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        component_registry.add_component<LoadoutComponent>(agent, loadout);
        
        // Update possession system to refresh agent mappings
        possession_system.update(0.016f);
        
        // Create obstacle outside range
        EntityID obstacle_entity = entity_manager.create_entity();
        Transform obstacle_transform{200.0f, 200.0f, 0.0f, 1.0f, 1.0f}; // ~141 pixels away
        InteractableComponent obstacle_interactable;
        obstacle_interactable.type = InteractionType::Tree;
        obstacle_interactable.interaction_radius = 48.0f; // Should be out of range
        
        component_registry.add_component<Transform>(obstacle_entity, obstacle_transform);
        component_registry.add_component<InteractableComponent>(obstacle_entity, obstacle_interactable);
        
        // Register obstacle and possess agent
        obstacle_system.register_obstacle(obstacle_entity);
        possession_system.possess_agent(1);
        
        // Update system to detect proximity
        obstacle_system.update(0.016f);
        
        // Agent should not detect the distant obstacle
        REQUIRE(obstacle_system.get_nearby_interactable() == INVALID_ENTITY);
    }
}