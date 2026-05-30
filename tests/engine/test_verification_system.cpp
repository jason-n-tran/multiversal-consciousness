#include <catch2/catch_test_macros.hpp>
#include "engine/VerificationSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/Components.h"

TEST_CASE("VerificationSystem Basic Functionality", "[verification][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    VerificationSystem verification_system(&entity_manager, &component_registry);
    
    SECTION("Initial state") {
        REQUIRE_FALSE(verification_system.is_scenario_completed());
        REQUIRE_FALSE(verification_system.is_scenario_failed());
        REQUIRE(verification_system.get_feedback_timer() > 0.0f);
    }
    
    SECTION("Reset functionality") {
        verification_system.trigger_reset();
        REQUIRE_FALSE(verification_system.is_scenario_completed());
        REQUIRE_FALSE(verification_system.is_scenario_failed());
        REQUIRE(verification_system.get_feedback_timer() > 0.0f);
    }
    
    SECTION("Update without entities") {
        verification_system.update(1.0f);
        REQUIRE_FALSE(verification_system.is_scenario_completed());
        REQUIRE_FALSE(verification_system.is_scenario_failed());
    }
}

TEST_CASE("VerificationSystem Success Detection", "[verification][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    VerificationSystem verification_system(&entity_manager, &component_registry);
    
    SECTION("Success zone detection") {
        // Create agent entity
        EntityID agent = entity_manager.create_entity();
        
        Agent agent_comp;
        agent_comp.agent_number = 1;
        agent_comp.is_possessed = true;
        component_registry.add_component(agent, std::move(agent_comp));
        
        Transform agent_transform;
        agent_transform.x = 520.0f; // Inside success zone (500-600)
        agent_transform.y = 420.0f; // Inside success zone (400-450)
        component_registry.add_component(agent, std::move(agent_transform));
        
        LoadoutComponent loadout;
        loadout.current_ability = AbilityType::DoubleJump;
        component_registry.add_component(agent, std::move(loadout));
        
        // Create success zone trigger
        EntityID trigger = entity_manager.create_entity();
        
        Trigger trigger_comp;
        trigger_comp.trigger_type = "success_zone";
        trigger_comp.width = 100.0f;
        trigger_comp.height = 50.0f;
        component_registry.add_component(trigger, std::move(trigger_comp));
        
        Transform trigger_transform;
        trigger_transform.x = 500.0f;
        trigger_transform.y = 400.0f;
        component_registry.add_component(trigger, std::move(trigger_transform));
        
        // Update verification system
        verification_system.update(1.0f);
        
        // Should complete successfully
        REQUIRE(verification_system.is_scenario_completed());
        REQUIRE_FALSE(verification_system.is_scenario_failed());
    }
}

TEST_CASE("VerificationSystem Feedback Messages", "[verification][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    VerificationSystem verification_system(&entity_manager, &component_registry);
    
    SECTION("Initial feedback message") {
        const std::string& message = verification_system.get_feedback_message();
        REQUIRE_FALSE(message.empty());
        REQUIRE(message.find("Verification Scenario") != std::string::npos);
    }
    
    SECTION("Verification stats") {
        std::string stats = verification_system.get_verification_stats();
        REQUIRE_FALSE(stats.empty());
        REQUIRE(stats.find("Verification Stats") != std::string::npos);
        REQUIRE(stats.find("Success Zone") != std::string::npos);
        REQUIRE(stats.find("Required Ability") != std::string::npos);
        REQUIRE(stats.find("Correct Reality") != std::string::npos);
    }
}