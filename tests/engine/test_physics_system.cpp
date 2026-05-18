#include <catch2/catch_test_macros.hpp>
#include "engine/PhysicsSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/Components.h"

TEST_CASE("PhysicsSystem - Basic Functionality", "[physics][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    PhysicsSystem physics_system;
    
    // Initialize the physics system
    physics_system.initialize(entity_manager, component_registry);
    
    SECTION("PhysicsSystem initializes correctly") {
        // Test that the system initializes without errors
        REQUIRE(true); // If we get here, initialization succeeded
    }
    
    SECTION("PhysicsSystem applies gravity to non-grounded entities") {
        // Create an entity with physics and transform components
        EntityID entity = entity_manager.create_entity();
        
        Transform transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent physics{0.0f, 0.0f, 0.0f, 980.0f, 1.0f, false, false, true, 0.8f, 0.0f};
        
        component_registry.add_component<Transform>(entity, transform);
        component_registry.add_component<PhysicsComponent>(entity, physics);
        
        // Update physics for one frame (0.016s = ~60fps)
        float delta_time = 0.016f;
        physics_system.update(delta_time);
        
        // Check that gravity was applied
        const PhysicsComponent* updated_physics = component_registry.get_component<PhysicsComponent>(entity);
        REQUIRE(updated_physics != nullptr);
        REQUIRE(updated_physics->velocity_y > 0.0f); // Gravity should increase downward velocity
    }
    
    SECTION("PhysicsSystem does not apply gravity to grounded entities") {
        // Create an entity with physics and transform components
        EntityID entity = entity_manager.create_entity();
        
        Transform transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent physics{0.0f, 0.0f, 0.0f, 980.0f, 1.0f, true, false, true, 0.8f, 0.0f}; // is_grounded = true
        
        component_registry.add_component<Transform>(entity, transform);
        component_registry.add_component<PhysicsComponent>(entity, physics);
        
        // Update physics for one frame
        float delta_time = 0.016f;
        physics_system.update(delta_time);
        
        // Check that gravity was not applied
        const PhysicsComponent* updated_physics = component_registry.get_component<PhysicsComponent>(entity);
        REQUIRE(updated_physics != nullptr);
        REQUIRE(updated_physics->velocity_y == 0.0f); // Velocity should remain zero
    }
    
    SECTION("PhysicsSystem updates position based on velocity") {
        // Create an entity with physics and transform components
        EntityID entity = entity_manager.create_entity();
        
        Transform transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent physics{50.0f, -30.0f, 0.0f, 0.0f, 1.0f, true, false, false, 0.8f, 0.0f}; // Moving right and up
        
        component_registry.add_component<Transform>(entity, transform);
        component_registry.add_component<PhysicsComponent>(entity, physics);
        
        // Update physics for one frame
        float delta_time = 0.1f; // 0.1 seconds for easier calculation
        physics_system.update(delta_time);
        
        // Check that position was updated based on velocity
        const Transform* updated_transform = component_registry.get_component<Transform>(entity);
        REQUIRE(updated_transform != nullptr);
        REQUIRE(std::abs(updated_transform->x - 105.0f) < 0.1f); // 100 + (50 * 0.1)
        REQUIRE(std::abs(updated_transform->y - 97.0f) < 0.1f);  // 100 + (-30 * 0.1)
    }
    
    physics_system.shutdown();
}

TEST_CASE("PhysicsSystem - AABB Collision Detection", "[physics][collision]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    PhysicsSystem physics_system;
    
    physics_system.initialize(entity_manager, component_registry);
    
    SECTION("PhysicsSystem detects collision between two entities") {
        // Create two entities with overlapping bounding boxes
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        // Entity 1 at (0, 0) with 32x32 bounding box
        Transform transform1{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent bbox1{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        // Entity 2 at (16, 16) with 32x32 bounding box (overlapping with entity1)
        Transform transform2{16.0f, 16.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent bbox2{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        component_registry.add_component<Transform>(entity1, transform1);
        component_registry.add_component<BoundingBoxComponent>(entity1, bbox1);
        component_registry.add_component<Transform>(entity2, transform2);
        component_registry.add_component<BoundingBoxComponent>(entity2, bbox2);
        
        // Test collision detection
        std::vector<CollisionInfo> collisions = physics_system.detect_collisions();
        
        // Should detect collision between the two entities
        REQUIRE(collisions.size() == 2); // Two collision infos (one for each entity)
        
        // Check first collision
        bool found_collision = false;
        for (const auto& collision : collisions) {
            if (collision.has_collision && collision.other_entity == entity2) {
                found_collision = true;
                // Since both overlaps are equal (16x16), it should be detected as vertical collision
                REQUIRE((collision.penetration_x > 0.0f || collision.penetration_y > 0.0f));
                break;
            }
        }
        REQUIRE(found_collision);
    }
    
    SECTION("PhysicsSystem does not detect collision between non-overlapping entities") {
        // Create two entities with non-overlapping bounding boxes
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        // Entity 1 at (0, 0) with 32x32 bounding box
        Transform transform1{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent bbox1{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        // Entity 2 at (100, 100) with 32x32 bounding box (far from entity1)
        Transform transform2{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent bbox2{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        component_registry.add_component<Transform>(entity1, transform1);
        component_registry.add_component<BoundingBoxComponent>(entity1, bbox1);
        component_registry.add_component<Transform>(entity2, transform2);
        component_registry.add_component<BoundingBoxComponent>(entity2, bbox2);
        
        // Test collision detection
        std::vector<CollisionInfo> collisions = physics_system.detect_collisions();
        
        // Should not detect any collisions
        REQUIRE(collisions.empty());
    }
    
    physics_system.shutdown();
}

TEST_CASE("PhysicsSystem - Collision Resolution with Slide Mechanics", "[physics][collision][slide]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    PhysicsSystem physics_system;
    
    physics_system.initialize(entity_manager, component_registry);
    
    SECTION("PhysicsSystem resolves horizontal collision with slide mechanics") {
        // Create a moving entity and a static wall
        EntityID moving_entity = entity_manager.create_entity();
        EntityID wall_entity = entity_manager.create_entity();
        
        // Moving entity positioned to collide with wall
        Transform moving_transform{30.0f, 50.0f, 0.0f, 1.0f, 1.0f};
        PhysicsComponent moving_physics{50.0f, 0.0f, 0.0f, 0.0f, 1.0f, false, false, false, 0.8f, 0.0f}; // No gravity
        BoundingBoxComponent moving_bbox{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        // Static wall entity - positioned to overlap with moving entity
        Transform wall_transform{50.0f, 50.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent wall_bbox{32.0f, 64.0f, 0.0f, 0.0f, true, false};
        
        component_registry.add_component<Transform>(moving_entity, moving_transform);
        component_registry.add_component<PhysicsComponent>(moving_entity, moving_physics);
        component_registry.add_component<BoundingBoxComponent>(moving_entity, moving_bbox);
        component_registry.add_component<Transform>(wall_entity, wall_transform);
        component_registry.add_component<BoundingBoxComponent>(wall_entity, wall_bbox);
        
        // Move the entity to create collision - small time step to ensure collision
        float delta_time = 0.1f;
        physics_system.update(delta_time);
        
        // Check that horizontal velocity was stopped or reduced due to collision
        const PhysicsComponent* updated_physics = component_registry.get_component<PhysicsComponent>(moving_entity);
        const Transform* updated_transform = component_registry.get_component<Transform>(moving_entity);
        
        REQUIRE(updated_physics != nullptr);
        REQUIRE(updated_transform != nullptr);
        
        // The entity should have moved but collision should have been resolved
        // Entity moved from X=30 to X=35, then was pushed back by penetration amount
        // With normal (-1, 0) and penetration 17, new X = 35 + (-1) * 17 = 18
        REQUIRE(updated_transform->x < 30.0f); // Should be pushed back to the left of its starting position
    }
    
    SECTION("PhysicsSystem sets grounded state on vertical collision from above") {
        // Create a falling entity and a ground entity
        EntityID falling_entity = entity_manager.create_entity();
        EntityID ground_entity = entity_manager.create_entity();
        
        // Falling entity positioned directly above ground with sufficient gap
        Transform falling_transform{50.0f, 20.0f, 0.0f, 1.0f, 1.0f}; // Moved higher up
        PhysicsComponent falling_physics{0.0f, 100.0f, 0.0f, 0.0f, 1.0f, false, false, false, 0.8f, 0.0f}; // Downward velocity
        BoundingBoxComponent falling_bbox{32.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        // Ground entity (static, no physics)
        Transform ground_transform{50.0f, 100.0f, 0.0f, 1.0f, 1.0f}; // Moved lower
        BoundingBoxComponent ground_bbox{64.0f, 32.0f, 0.0f, 0.0f, true, false};
        
        component_registry.add_component<Transform>(falling_entity, falling_transform);
        component_registry.add_component<PhysicsComponent>(falling_entity, falling_physics);
        component_registry.add_component<BoundingBoxComponent>(falling_entity, falling_bbox);
        component_registry.add_component<Transform>(ground_entity, ground_transform);
        component_registry.add_component<BoundingBoxComponent>(ground_entity, ground_bbox);
        
        // Check initial state
        const PhysicsComponent* initial_physics = component_registry.get_component<PhysicsComponent>(falling_entity);
        const Transform* initial_transform = component_registry.get_component<Transform>(falling_entity);
        REQUIRE(initial_physics->velocity_y == 100.0f);
        REQUIRE(initial_transform->y == 20.0f);
        
        // Check if there's any collision initially (there shouldn't be)
        std::vector<CollisionInfo> initial_collisions = physics_system.detect_collisions();
        REQUIRE(initial_collisions.empty());
        
        // Update physics one step at a time and check velocity/position
        float delta_time = 0.1f;
        physics_system.update(delta_time);
        
        // Check position and velocity after first update
        const Transform* transform_after_1 = component_registry.get_component<Transform>(falling_entity);
        const PhysicsComponent* physics_after_1 = component_registry.get_component<PhysicsComponent>(falling_entity);
        
        REQUIRE(transform_after_1 != nullptr);
        REQUIRE(physics_after_1 != nullptr);
        
        // Check if collision was detected after first update (shouldn't be)
        std::vector<CollisionInfo> collisions_after_1 = physics_system.detect_collisions();
        REQUIRE(collisions_after_1.empty()); // Should still be no collision
        
        // After 0.1s with velocity 100, should move 10 pixels: Y = 20 + 10 = 30
        REQUIRE(std::abs(transform_after_1->y - 30.0f) < 0.1f);
        REQUIRE(physics_after_1->velocity_y == 100.0f); // Velocity should remain the same
        
        // Continue updating until collision should occur
        for (int i = 1; i < 5; ++i) {
            physics_system.update(delta_time);
        }
        
        // Debug: Check the final position of the falling entity
        const Transform* final_transform = component_registry.get_component<Transform>(falling_entity);
        const PhysicsComponent* final_physics = component_registry.get_component<PhysicsComponent>(falling_entity);
        
        REQUIRE(final_transform != nullptr);
        REQUIRE(final_physics != nullptr);
        
        // After 5 updates of 0.1s each with velocity 100, entity should be at Y = 20 + (100 * 0.5) = 70
        REQUIRE(final_transform->y > 60.0f); // Should have moved past the initial gap
        
        // Should be grounded after collision with ground
        REQUIRE(final_physics->is_grounded == true);
    }
    
    physics_system.shutdown();
}