#include <catch2/catch_test_macros.hpp>
#include "engine/RealityManager.h"
#include "engine/RealitySystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"

TEST_CASE("RealityManager basic functionality", "[reality]") {
    RealityManager reality_manager;
    
    SECTION("Initial state is Reality A") {
        REQUIRE(reality_manager.get_current_reality() == Reality::A);
    }
    
    SECTION("Reality switching works") {
        REQUIRE(reality_manager.get_current_reality() == Reality::A);
        
        bool switch_success = reality_manager.switch_reality();
        REQUIRE(switch_success == true);
        REQUIRE(reality_manager.get_current_reality() == Reality::B);
        
        switch_success = reality_manager.switch_reality();
        REQUIRE(switch_success == true);
        REQUIRE(reality_manager.get_current_reality() == Reality::A);
    }
    
    SECTION("Shared geometry synchronization") {
        EntityID entity = 1;
        Transform transform{100.0f, 200.0f, 0.5f, 1.5f, 2.0f};
        
        reality_manager.sync_shared_geometry(entity, transform);
        
        const Transform* retrieved = reality_manager.get_shared_geometry(entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->x == 100.0f);
        REQUIRE(retrieved->y == 200.0f);
        REQUIRE(retrieved->rotation == 0.5f);
        REQUIRE(retrieved->scale_x == 1.5f);
        REQUIRE(retrieved->scale_y == 2.0f);
    }
    
    SECTION("Reality-specific inventory storage") {
        EntityID entity = 1;
        Inventory inventory_a, inventory_b;
        
        inventory_a.items.insert("key_a");
        inventory_a.abilities["strength"] = 5;
        
        inventory_b.items.insert("key_b");
        inventory_b.abilities["speed"] = 3;
        
        reality_manager.set_reality_inventory(entity, inventory_a, Reality::A);
        reality_manager.set_reality_inventory(entity, inventory_b, Reality::B);
        
        const Inventory* retrieved_a = reality_manager.get_reality_inventory(entity, Reality::A);
        const Inventory* retrieved_b = reality_manager.get_reality_inventory(entity, Reality::B);
        
        REQUIRE(retrieved_a != nullptr);
        REQUIRE(retrieved_b != nullptr);
        
        REQUIRE(retrieved_a->items.count("key_a") == 1);
        REQUIRE(retrieved_a->items.count("key_b") == 0);
        REQUIRE(retrieved_a->abilities.at("strength") == 5);
        
        REQUIRE(retrieved_b->items.count("key_b") == 1);
        REQUIRE(retrieved_b->items.count("key_a") == 0);
        REQUIRE(retrieved_b->abilities.at("speed") == 3);
    }
    
    SECTION("Shared door state synchronization") {
        EntityID door_entity = 1;
        Door door;
        door.is_open = true;
        door.is_locked = false;
        door.required_key = "master_key";
        door.animation_progress = 0.75f;
        
        reality_manager.sync_shared_door(door_entity, door);
        
        const Door* retrieved = reality_manager.get_shared_door(door_entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->is_open == true);
        REQUIRE(retrieved->is_locked == false);
        REQUIRE(retrieved->required_key == "master_key");
        REQUIRE(retrieved->animation_progress == 0.75f);
    }
    
    SECTION("Shared water level synchronization") {
        EntityID water_entity = 1;
        WaterLevel water;
        water.current_level = 50.0f;
        water.target_level = 75.0f;
        water.change_rate = 25.0f;
        water.is_filling = true;
        
        reality_manager.sync_shared_water_level(water_entity, water);
        
        const WaterLevel* retrieved = reality_manager.get_shared_water_level(water_entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->current_level == 50.0f);
        REQUIRE(retrieved->target_level == 75.0f);
        REQUIRE(retrieved->change_rate == 25.0f);
        REQUIRE(retrieved->is_filling == true);
        REQUIRE(retrieved->is_draining == false);
    }
    
    SECTION("Shared environmental switch synchronization") {
        EntityID switch_entity = 1;
        EnvironmentalSwitch env_switch;
        env_switch.is_activated = true;
        env_switch.target_entity_type = "door";
        env_switch.target_entity_id = "door_1";
        env_switch.requires_agent_presence = false;
        
        reality_manager.sync_shared_switch(switch_entity, env_switch);
        
        const EnvironmentalSwitch* retrieved = reality_manager.get_shared_switch(switch_entity);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->is_activated == true);
        REQUIRE(retrieved->target_entity_type == "door");
        REQUIRE(retrieved->target_entity_id == "door_1");
        REQUIRE(retrieved->requires_agent_presence == false);
    }
    
    SECTION("Entity removal cleans up all state") {
        EntityID entity = 1;
        
        // Add data to all storage types
        Transform transform{10.0f, 20.0f, 0.0f, 1.0f, 1.0f};
        reality_manager.sync_shared_geometry(entity, transform);
        
        Inventory inventory;
        inventory.items.insert("test_item");
        reality_manager.set_reality_inventory(entity, inventory, Reality::A);
        reality_manager.set_reality_inventory(entity, inventory, Reality::B);
        
        Door door;
        door.is_open = true;
        reality_manager.sync_shared_door(entity, door);
        
        // Verify data exists
        REQUIRE(reality_manager.get_shared_geometry(entity) != nullptr);
        REQUIRE(reality_manager.get_reality_inventory(entity, Reality::A) != nullptr);
        REQUIRE(reality_manager.get_reality_inventory(entity, Reality::B) != nullptr);
        REQUIRE(reality_manager.get_shared_door(entity) != nullptr);
        
        // Remove entity
        reality_manager.remove_entity(entity);
        
        // Verify all data is cleaned up
        REQUIRE(reality_manager.get_shared_geometry(entity) == nullptr);
        REQUIRE(reality_manager.get_reality_inventory(entity, Reality::A) == nullptr);
        REQUIRE(reality_manager.get_reality_inventory(entity, Reality::B) == nullptr);
        REQUIRE(reality_manager.get_shared_door(entity) == nullptr);
    }
}

TEST_CASE("RealitySystem integration", "[reality][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    RealitySystem reality_system;
    
    reality_system.initialize(entity_manager, component_registry);
    
    SECTION("System initializes correctly") {
        REQUIRE(reality_system.get_current_reality() == Reality::A);
    }
    
    SECTION("Reality switching through system") {
        REQUIRE(reality_system.get_current_reality() == Reality::A);
        
        bool switch_success = reality_system.switch_reality();
        REQUIRE(switch_success == true);
        REQUIRE(reality_system.get_current_reality() == Reality::B);
    }
    
    SECTION("Entity synchronization with components") {
        EntityID entity = entity_manager.create_entity();
        
        // Add components
        Transform transform{50.0f, 100.0f, 1.0f, 2.0f, 2.0f};
        component_registry.add_component<Transform>(entity, transform);
        
        Door door;
        door.is_open = false;
        door.required_key = "test_key";
        component_registry.add_component<Door>(entity, door);
        
        // Synchronize entity
        reality_system.synchronize_entity(entity);
        
        // Verify shared state is synchronized
        const Door* shared_door = reality_system.get_shared_door(entity);
        REQUIRE(shared_door != nullptr);
        REQUIRE(shared_door->is_open == false);
        REQUIRE(shared_door->required_key == "test_key");
    }
}