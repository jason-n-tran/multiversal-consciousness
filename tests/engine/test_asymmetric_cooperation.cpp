#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "../../src/engine/CoordinationSystem.h"
#include "../../src/engine/PuzzleSystem.h"
#include "../../src/engine/EntityManager.h"
#include "../../src/engine/ComponentRegistry.h"
#include "../../src/engine/RealityManager.h"

TEST_CASE("Asymmetric cooperation integration", "[integration][cooperation]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    auto puzzle_system = std::make_unique<PuzzleSystem>();
    
    // Initialize systems
    coordination_system->initialize(*entity_manager, *component_registry);
    puzzle_system->initialize(*entity_manager, *component_registry);
    puzzle_system->set_coordination_system(coordination_system.get());
    
    SECTION("Multi-agent coordination with puzzle completion") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        component_registry->add_component<Transform>(agent2, Transform{200.0f, 200.0f, 0.0f, 1.0f, 1.0f});
        
        // Create a puzzle that requires both agents to be in specific positions
        Puzzle coordination_puzzle("multi_agent_test", "Multi-Agent Coordination Test", true);
        
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::pair<float, float>> target_positions = {{100.0f, 100.0f}, {200.0f, 200.0f}};
        auto position_condition = puzzle_system->create_position_condition("positions", agents, target_positions, 16.0f);
        coordination_puzzle.conditions.push_back(position_condition);
        
        puzzle_system->register_puzzle(coordination_puzzle);
        
        // Register the puzzle scenario with coordination system
        coordination_system->register_puzzle_scenario("multi_agent_test", agents);
        
        // Initially puzzle should not be complete
        REQUIRE_FALSE(puzzle_system->is_puzzle_complete("multi_agent_test"));
        REQUIRE_FALSE(coordination_system->is_puzzle_complete("multi_agent_test"));
        
        // Update systems to check conditions
        puzzle_system->update(0.016f);
        coordination_system->update(0.016f);
        
        // Since agents are already in target positions, puzzle should be complete
        REQUIRE(puzzle_system->is_puzzle_complete("multi_agent_test"));
        
        // Check that completed puzzles list is updated
        const auto& completed = puzzle_system->get_completed_puzzles();
        REQUIRE(completed.size() == 1);
        REQUIRE(completed[0] == "multi_agent_test");
    }
    
    SECTION("Agent actions generate effects and feedback") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        component_registry->add_component<Transform>(agent2, Transform{120.0f, 100.0f, 0.0f, 1.0f, 1.0f}); // Close to agent1
        
        // Record an action that should affect nearby agents
        coordination_system->record_agent_action(agent1, "movement", Reality::A);
        
        // Update coordination system to process effects
        coordination_system->update(0.016f);
        
        // Generate feedback for the action
        std::vector<EntityID> affected_agents = {agent2};
        puzzle_system->generate_action_feedback(agent1, affected_agents, "movement");
        
        // Check that feedback was generated
        const auto& feedback = puzzle_system->get_active_feedback();
        REQUIRE(feedback.size() == 1);
        REQUIRE(feedback[0].source_agent == agent1);
        REQUIRE(feedback[0].affected_agents.size() == 1);
        REQUIRE(feedback[0].affected_agents[0] == agent2);
        
        // Update puzzle system to manage feedback
        puzzle_system->update(0.016f);
        
        // Feedback should still be active (within display duration)
        REQUIRE(puzzle_system->get_active_feedback().size() == 1);
    }
    
    SECTION("Timing mechanics across reality switches") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        std::vector<EntityID> required_agents = {agent1, agent2};
        auto time_window = std::chrono::milliseconds(1000);
        auto completion_check = []() { return false; }; // Never complete for this test
        
        // Start timing mechanic
        bool started = coordination_system->start_timing_mechanic("reality_switch_test", required_agents, time_window, completion_check);
        REQUIRE(started);
        
        // Verify mechanic is active
        const auto& mechanics = coordination_system->get_active_mechanics();
        REQUIRE(mechanics.size() == 1);
        REQUIRE(mechanics.at("reality_switch_test").is_active);
        
        // Simulate reality switch
        coordination_system->handle_reality_switch();
        
        // Mechanic should still be active after reality switch
        const auto& mechanics_after_switch = coordination_system->get_active_mechanics();
        REQUIRE(mechanics_after_switch.size() == 1);
        REQUIRE(mechanics_after_switch.at("reality_switch_test").is_active);
        
        // Update timing mechanics
        coordination_system->update(0.016f);
        
        // Mechanic should still be active (within time window)
        REQUIRE(coordination_system->get_active_mechanics().at("reality_switch_test").is_active);
    }
}