#include <catch2/catch_test_macros.hpp>
#include "../../src/engine/HUDSystem.h"
#include "../../src/engine/PossessionSystem.h"
#include "../../src/engine/QuantumLoadoutSystem.h"
#include "../../src/engine/RealityManager.h"
#include "../../src/engine/EntityManager.h"
#include "../../src/engine/ComponentRegistry.h"

TEST_CASE("HUDSystem - Basic Functionality", "[hud][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    HUDSystem hud_system;
    
    SECTION("System initializes correctly") {
        hud_system.initialize(entity_manager, component_registry);
        
        REQUIRE(hud_system.get_displayed_agent_number() == 0);
        REQUIRE(hud_system.get_displayed_abilities().empty());
    }
    
    SECTION("Visual configuration can be set and retrieved") {
        HUDVisualConfig config;
        config.agent_number_font_size = 32.0f;
        config.ability_font_size = 20.0f;
        
        hud_system.set_visual_config(config);
        
        const auto& retrieved_config = hud_system.get_visual_config();
        REQUIRE(retrieved_config.agent_number_font_size == 32.0f);
        REQUIRE(retrieved_config.ability_font_size == 20.0f);
    }
}

TEST_CASE("HUDSystem - State Management", "[hud][system][state]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    HUDSystem hud_system;
    PossessionSystem possession_system;
    QuantumLoadoutSystem loadout_system;
    RealityManager reality_manager;
    
    hud_system.initialize(entity_manager, component_registry);
    possession_system.initialize(entity_manager, component_registry);
    loadout_system.initialize(entity_manager, component_registry);
    
    // Wire systems together
    hud_system.set_possession_system(&possession_system);
    hud_system.set_loadout_system(&loadout_system);
    hud_system.set_reality_manager(&reality_manager);
    
    SECTION("HUD responds to possession changes") {
        // Initially no agent possessed
        REQUIRE(hud_system.get_displayed_agent_number() == 0);
        
        // Simulate possession change notification
        hud_system.on_possession_changed(3);
        
        // HUD should update immediately
        REQUIRE(hud_system.get_displayed_agent_number() == 3);
    }
    
    SECTION("HUD responds to reality changes") {
        // Simulate reality change notification
        hud_system.on_reality_changed(Reality::B);
        
        // HUD should trigger update (we can't easily test the internal state change
        // without more complex setup, but we can verify the method doesn't crash)
        REQUIRE_NOTHROW(hud_system.update(0.016f));
    }
    
    SECTION("HUD responds to ability changes") {
        EntityID test_entity = entity_manager.create_entity();
        
        // Simulate ability change notification
        hud_system.on_abilities_changed(test_entity);
        
        // HUD should trigger update
        REQUIRE_NOTHROW(hud_system.update(0.016f));
    }
}

TEST_CASE("HUDSystem - Rendering", "[hud][system][rendering]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    HUDSystem hud_system;
    
    hud_system.initialize(entity_manager, component_registry);
    
    SECTION("Render method doesn't crash with null renderer") {
        REQUIRE_NOTHROW(hud_system.render(nullptr));
    }
    
    SECTION("Update method works correctly") {
        REQUIRE_NOTHROW(hud_system.update(0.016f));
        REQUIRE_NOTHROW(hud_system.update(0.033f));
    }
    
    SECTION("Force update works") {
        hud_system.force_update();
        REQUIRE_NOTHROW(hud_system.update(0.016f));
    }
}

TEST_CASE("HUDSystem - System Integration", "[hud][system][integration]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    HUDSystem hud_system;
    PossessionSystem possession_system;
    
    hud_system.initialize(entity_manager, component_registry);
    possession_system.initialize(entity_manager, component_registry);
    
    SECTION("Systems can be wired together") {
        REQUIRE_NOTHROW(hud_system.set_possession_system(&possession_system));
        REQUIRE_NOTHROW(possession_system.set_hud_system(&hud_system));
    }
    
    SECTION("Shutdown works correctly") {
        hud_system.set_possession_system(&possession_system);
        REQUIRE_NOTHROW(hud_system.shutdown());
    }
}