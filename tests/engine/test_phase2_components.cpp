#include <catch2/catch_test_macros.hpp>
#include "engine/Components.h"
#include "engine/AbilitySystem.h"
#include "engine/PhysicsTypes.h"
#include "engine/IInteractable.h"

TEST_CASE("Phase 2 Components - Basic Structure", "[components][phase2]") {
    SECTION("PhysicsComponent has correct default values") {
        PhysicsComponent physics;
        
        REQUIRE(physics.velocity_x == 0.0f);
        REQUIRE(physics.velocity_y == 0.0f);
        REQUIRE(physics.acceleration_x == 0.0f);
        REQUIRE(physics.acceleration_y == 980.0f);  // Gravity
        REQUIRE(physics.mass == 1.0f);
        REQUIRE(physics.is_grounded == false);
        REQUIRE(physics.apply_gravity == true);
        REQUIRE(physics.friction == 0.8f);
        REQUIRE(physics.bounce == 0.0f);
    }
    
    SECTION("BoundingBoxComponent has correct default values") {
        BoundingBoxComponent bbox;
        
        REQUIRE(bbox.width == 32.0f);
        REQUIRE(bbox.height == 32.0f);
        REQUIRE(bbox.offset_x == 0.0f);
        REQUIRE(bbox.offset_y == 0.0f);
        REQUIRE(bbox.is_solid == true);
        REQUIRE(bbox.is_trigger == false);
    }
    
    SECTION("LoadoutComponent has correct default values") {
        LoadoutComponent loadout;
        
        REQUIRE(loadout.current_ability == AbilityType::None);
        REQUIRE(loadout.ability_cooldown == 0.0f);
        REQUIRE(loadout.ability_uses == 0);
        REQUIRE(loadout.ability_ready == true);
        REQUIRE(loadout.reality_abilities.empty());
    }
    
    SECTION("InteractableComponent has correct default values") {
        InteractableComponent interactable;
        
        REQUIRE(interactable.type == InteractionType::Tree);
        REQUIRE(interactable.required_ability == AbilityType::None);
        REQUIRE(interactable.is_active == true);
        REQUIRE(interactable.interaction_radius == 48.0f);
        REQUIRE(interactable.interaction_text == "Press E to interact");
        REQUIRE(interactable.linked_entity == 0);
    }
}

TEST_CASE("AbilityType Enum Values", "[abilities][phase2]") {
    SECTION("AbilityType enum has expected values") {
        REQUIRE(static_cast<int>(AbilityType::None) == 0);
        REQUIRE(static_cast<int>(AbilityType::Axe) == 1);
        REQUIRE(static_cast<int>(AbilityType::Keycard) == 2);
        REQUIRE(static_cast<int>(AbilityType::DoubleJump) == 3);
        REQUIRE(static_cast<int>(AbilityType::Dash) == 4);
        REQUIRE(static_cast<int>(AbilityType::WaterWalk) == 5);
        REQUIRE(static_cast<int>(AbilityType::PhaseShift) == 6);
    }
}

TEST_CASE("InteractionType Enum Values", "[interactions][phase2]") {
    SECTION("InteractionType enum has expected values") {
        REQUIRE(static_cast<int>(InteractionType::Tree) == 0);
        REQUIRE(static_cast<int>(InteractionType::Door) == 1);
        REQUIRE(static_cast<int>(InteractionType::Chasm) == 2);
        REQUIRE(static_cast<int>(InteractionType::Switch) == 3);
        REQUIRE(static_cast<int>(InteractionType::QuantumNode) == 4);
    }
}

TEST_CASE("AbilityRegistry Basic Functionality", "[abilities][registry][phase2]") {
    SECTION("AbilityRegistry can register and retrieve abilities") {
        AbilityRegistry registry;
        
        AbilityDefinition axe_ability{
            AbilityType::Axe,
            "Axe",
            "Chop down trees",
            0.5f,
            -1,
            false,
            0.0f
        };
        
        registry.register_ability(axe_ability);
        
        const AbilityDefinition* retrieved = registry.get_ability(AbilityType::Axe);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->type == AbilityType::Axe);
        REQUIRE(retrieved->name == "Axe");
        REQUIRE(retrieved->description == "Chop down trees");
        REQUIRE(retrieved->cooldown_time == 0.5f);
        REQUIRE(retrieved->max_uses == -1);
        REQUIRE(retrieved->requires_ground == false);
        REQUIRE(retrieved->energy_cost == 0.0f);
    }
    
    SECTION("AbilityRegistry returns nullptr for unregistered abilities") {
        AbilityRegistry registry;
        
        const AbilityDefinition* retrieved = registry.get_ability(AbilityType::Dash);
        REQUIRE(retrieved == nullptr);
    }
    
    SECTION("AbilityRegistry initializes default abilities") {
        AbilityRegistry registry;
        registry.initialize_default_abilities();
        
        // Check that all default abilities are registered
        REQUIRE(registry.get_ability(AbilityType::None) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::Axe) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::Keycard) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::DoubleJump) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::Dash) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::WaterWalk) != nullptr);
        REQUIRE(registry.get_ability(AbilityType::PhaseShift) != nullptr);
    }
}

TEST_CASE("AABB Collision Detection", "[physics][collision][phase2]") {
    SECTION("AABB intersects method works correctly") {
        AABB box1{0.0f, 0.0f, 10.0f, 10.0f};
        AABB box2{5.0f, 5.0f, 15.0f, 15.0f};
        AABB box3{20.0f, 20.0f, 30.0f, 30.0f};
        
        REQUIRE(box1.intersects(box2) == true);
        REQUIRE(box2.intersects(box1) == true);
        REQUIRE(box1.intersects(box3) == false);
        REQUIRE(box3.intersects(box1) == false);
    }
    
    SECTION("AABB overlap calculations work correctly") {
        AABB box1{0.0f, 0.0f, 10.0f, 10.0f};
        AABB box2{5.0f, 5.0f, 15.0f, 15.0f};
        
        float overlap_x = box1.get_overlap_x(box2);
        float overlap_y = box1.get_overlap_y(box2);
        
        REQUIRE(overlap_x == 5.0f);
        REQUIRE(overlap_y == 5.0f);
    }
    
    SECTION("AABB from_components creates correct bounds") {
        Transform transform{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
        BoundingBoxComponent bbox{32.0f, 64.0f, 0.0f, 0.0f};
        
        AABB aabb = AABB::from_components(transform, bbox);
        
        REQUIRE(aabb.min_x == 84.0f);  // 100 - 16
        REQUIRE(aabb.min_y == 168.0f); // 200 - 32
        REQUIRE(aabb.max_x == 116.0f); // 100 + 16
        REQUIRE(aabb.max_y == 232.0f); // 200 + 32
    }
}

TEST_CASE("CollisionInfo Structure", "[physics][collision][phase2]") {
    SECTION("CollisionInfo has correct default values") {
        CollisionInfo collision;
        
        REQUIRE(collision.has_collision == false);
        REQUIRE(collision.penetration_x == 0.0f);
        REQUIRE(collision.penetration_y == 0.0f);
        REQUIRE(collision.normal_x == 0.0f);
        REQUIRE(collision.normal_y == 0.0f);
        REQUIRE(collision.other_entity == 0);
    }
}