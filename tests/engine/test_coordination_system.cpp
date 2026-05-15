#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "engine/CoordinationSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/RealityManager.h"
#include "engine/Components.h"

TEST_CASE("CoordinationSystem basic functionality", "[coordination]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    
    // Create coordination system
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    
    SECTION("System can be created and initialized") {
        // Just test that the system can be created and initialized without crashing
        REQUIRE(coordination_system != nullptr);
        
        // Initialize system
        coordination_system->initialize(*entity_manager, *component_registry);
        
        // Basic functionality test
        REQUIRE(coordination_system->get_recent_actions().empty());
        REQUIRE(coordination_system->get_pending_effects().empty());
        REQUIRE(coordination_system->get_active_mechanics().empty());
    }
    
    SECTION("Record and retrieve agent actions") {
        // Initialize system first
        coordination_system->initialize(*entity_manager, *component_registry);
        
        // Create test agent
        EntityID agent = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent, Agent{1, false, 100.0f});
        component_registry->add_component<Transform>(agent, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        
        // Record an action
        std::unordered_map<std::string, float> params;
        params["speed"] = 50.0f;
        coordination_system->record_agent_action(agent, "movement", Reality::A, params);
        
        // Check that action was recorded
        const auto& actions = coordination_system->get_recent_actions();
        REQUIRE(actions.size() == 1);
        REQUIRE(actions[0].agent_entity == agent);
        REQUIRE(actions[0].action_type == "movement");
        REQUIRE(actions[0].action_reality == Reality::A);
        REQUIRE(actions[0].parameters.at("speed") == 50.0f);
    }
    
    SECTION("Register and check puzzle scenarios") {
        // Initialize system first
        coordination_system->initialize(*entity_manager, *component_registry);
        
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        
        // Register puzzle scenario
        std::vector<EntityID> participants = {agent1, agent2};
        coordination_system->register_puzzle_scenario("test_puzzle", participants);
        
        // Initially puzzle should not be complete
        REQUIRE_FALSE(coordination_system->is_puzzle_complete("test_puzzle"));
    }
    
    SECTION("Start and manage timing mechanics") {
        // Initialize system first
        coordination_system->initialize(*entity_manager, *component_registry);
        
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        std::vector<EntityID> required_agents = {agent1, agent2};
        auto time_window = std::chrono::milliseconds(1000);
        
        // Simple completion check that always returns false
        auto completion_check = []() { return false; };
        
        // Start timing mechanic
        bool started = coordination_system->start_timing_mechanic("test_mechanic", required_agents, time_window, completion_check);
        REQUIRE(started);
        
        // Check that mechanic is active
        const auto& mechanics = coordination_system->get_active_mechanics();
        REQUIRE(mechanics.size() == 1);
        REQUIRE(mechanics.at("test_mechanic").is_active);
        REQUIRE(mechanics.at("test_mechanic").time_window == time_window);
    }
    
    SECTION("Update system processes effects") {
        // Initialize system first
        coordination_system->initialize(*entity_manager, *component_registry);
        
        // Create test agent
        EntityID agent = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent, Agent{1, false, 100.0f});
        component_registry->add_component<Transform>(agent, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        
        // Record an action
        coordination_system->record_agent_action(agent, "movement", Reality::A);
        
        // Update system (should process effects)
        coordination_system->update(0.016f); // 16ms frame time
        
        // System should have processed the action
        // (Specific effects depend on the implementation details)
        REQUIRE(coordination_system->get_recent_actions().size() == 1);
    }
}

TEST_CASE("CoordinationSystem inter-agent effects", "[coordination]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    
    // Create coordination system
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    coordination_system->initialize(*entity_manager, *component_registry);
    
    SECTION("Movement effects between nearby agents") {
        // Create two agents close to each other
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        component_registry->add_component<Transform>(agent2, Transform{120.0f, 100.0f, 0.0f, 1.0f, 1.0f}); // 20 units away
        
        // Record movement action for agent1
        coordination_system->record_agent_action(agent1, "movement", Reality::A);
        
        // Update system to process effects
        coordination_system->update(0.016f);
        
        // Should have generated effects for nearby agent
        // Effects are processed and cleared in update, so we can't directly test them here
        // In a real test, you might want to expose more internal state or use a test observer
        REQUIRE(coordination_system->get_recent_actions().size() == 1);
    }
}

TEST_CASE("CoordinationSystem reality switch handling", "[coordination]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    
    // Create coordination system
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    coordination_system->initialize(*entity_manager, *component_registry);
    
    SECTION("Handle reality switch for timing mechanics") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        std::vector<EntityID> required_agents = {agent1, agent2};
        auto time_window = std::chrono::milliseconds(1000);
        auto completion_check = []() { return false; };
        
        // Start timing mechanic
        coordination_system->start_timing_mechanic("test_mechanic", required_agents, time_window, completion_check);
        
        // Handle reality switch
        coordination_system->handle_reality_switch();
        
        // Mechanic should still be active after reality switch
        const auto& mechanics = coordination_system->get_active_mechanics();
        REQUIRE(mechanics.size() == 1);
        REQUIRE(mechanics.at("test_mechanic").is_active);
    }
}