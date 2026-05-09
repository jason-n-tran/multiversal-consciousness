#include <catch2/catch_test_macros.hpp>
#include "engine/Components.h"
#include "engine/ComponentRegistry.h"
#include "engine/EntityManager.h"

TEST_CASE("Core game components can be created and used with ECS", "[components]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    
    // Create test entity
    EntityID entity = entity_manager.create_entity();
    REQUIRE(entity_manager.is_valid(entity));
    
    SECTION("Transform component") {
        Transform transform;
        transform.x = 100.0f;
        transform.y = 200.0f;
        transform.rotation = 1.57f; // 90 degrees
        transform.scale_x = 2.0f;
        transform.scale_y = 1.5f;
        
        component_registry.add_component<Transform>(entity, transform);
        
        REQUIRE(component_registry.has_component<Transform>(entity));
        
        const Transform* retrieved = component_registry.get_component<Transform>(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->x == 100.0f);
        REQUIRE(retrieved->y == 200.0f);
        REQUIRE(retrieved->rotation == 1.57f);
        REQUIRE(retrieved->scale_x == 2.0f);
        REQUIRE(retrieved->scale_y == 1.5f);
    }
    
    SECTION("Renderable component") {
        Renderable renderable;
        renderable.texture_id = "player_sprite";
        renderable.source_rect = {0, 0, 64, 64};
        renderable.color_r = 0.8f;
        renderable.color_g = 0.6f;
        renderable.color_b = 0.4f;
        renderable.color_a = 0.9f;
        renderable.layer = 5;
        
        component_registry.add_component<Renderable>(entity, renderable);
        
        REQUIRE(component_registry.has_component<Renderable>(entity));
        
        const Renderable* retrieved = component_registry.get_component<Renderable>(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->texture_id == "player_sprite");
        REQUIRE(retrieved->source_rect.w == 64);
        REQUIRE(retrieved->source_rect.h == 64);
        REQUIRE(retrieved->color_r == 0.8f);
        REQUIRE(retrieved->layer == 5);
    }
    
    SECTION("Agent component") {
        Agent agent;
        agent.agent_number = 3;
        agent.is_possessed = true;
        agent.movement_speed = 150.0f;
        
        component_registry.add_component<Agent>(entity, agent);
        
        REQUIRE(component_registry.has_component<Agent>(entity));
        
        const Agent* retrieved = component_registry.get_component<Agent>(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->agent_number == 3);
        REQUIRE(retrieved->is_possessed == true);
        REQUIRE(retrieved->movement_speed == 150.0f);
    }
    
    SECTION("Inventory component") {
        Inventory inventory;
        inventory.items.insert("keycard");
        inventory.items.insert("emp_device");
        inventory.abilities["hacking"] = 3;
        inventory.abilities["stealth"] = 1;
        
        component_registry.add_component<Inventory>(entity, inventory);
        
        REQUIRE(component_registry.has_component<Inventory>(entity));
        
        const Inventory* retrieved = component_registry.get_component<Inventory>(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->items.count("keycard") == 1);
        REQUIRE(retrieved->items.count("emp_device") == 1);
        REQUIRE(retrieved->items.count("nonexistent") == 0);
        REQUIRE(retrieved->abilities.at("hacking") == 3);
        REQUIRE(retrieved->abilities.at("stealth") == 1);
    }
    
    SECTION("QuantumNode component") {
        QuantumNode quantum_node;
        quantum_node.reality_a_item = "keycard";
        quantum_node.reality_b_item = "emp_device";
        quantum_node.is_activated = false;
        quantum_node.interaction_radius = 48.0f;
        
        component_registry.add_component<QuantumNode>(entity, quantum_node);
        
        REQUIRE(component_registry.has_component<QuantumNode>(entity));
        
        const QuantumNode* retrieved = component_registry.get_component<QuantumNode>(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->reality_a_item == "keycard");
        REQUIRE(retrieved->reality_b_item == "emp_device");
        REQUIRE(retrieved->is_activated == false);
        REQUIRE(retrieved->interaction_radius == 48.0f);
    }
    
    SECTION("Multiple components on same entity") {
        // Add multiple components to the same entity
        Transform transform{50.0f, 75.0f, 0.0f, 1.0f, 1.0f};
        Agent agent{5, false, 120.0f};
        Inventory inventory;
        inventory.items.insert("tool");
        
        component_registry.add_component<Transform>(entity, transform);
        component_registry.add_component<Agent>(entity, agent);
        component_registry.add_component<Inventory>(entity, inventory);
        
        // Verify all components exist
        REQUIRE(component_registry.has_component<Transform>(entity));
        REQUIRE(component_registry.has_component<Agent>(entity));
        REQUIRE(component_registry.has_component<Inventory>(entity));
        
        // Verify component data integrity
        const Transform* t = component_registry.get_component<Transform>(entity);
        const Agent* a = component_registry.get_component<Agent>(entity);
        const Inventory* i = component_registry.get_component<Inventory>(entity);
        
        REQUIRE(t->x == 50.0f);
        REQUIRE(a->agent_number == 5);
        REQUIRE(i->items.count("tool") == 1);
    }
}