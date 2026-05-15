#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <thread>
#include <chrono>
#include "engine/PuzzleSystem.h"
#include "engine/CoordinationSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/RealityManager.h"
#include "engine/Components.h"

TEST_CASE("PuzzleSystem basic functionality", "[puzzle]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    
    // Create puzzle system
    auto puzzle_system = std::make_unique<PuzzleSystem>();
    
    // Initialize systems
    coordination_system->initialize(*entity_manager, *component_registry);
    puzzle_system->initialize(*entity_manager, *component_registry);
    puzzle_system->set_coordination_system(coordination_system.get());
    
    SECTION("Register and check puzzle completion") {
        // Create a simple puzzle
        Puzzle test_puzzle("test_puzzle", "Test Puzzle", true);
        
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        component_registry->add_component<Transform>(agent2, Transform{200.0f, 200.0f, 0.0f, 1.0f, 1.0f});
        
        // Add a position condition
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::pair<float, float>> target_positions = {{100.0f, 100.0f}, {200.0f, 200.0f}};
        auto condition = puzzle_system->create_position_condition("pos_condition", agents, target_positions, 16.0f);
        test_puzzle.conditions.push_back(condition);
        
        // Register puzzle
        puzzle_system->register_puzzle(test_puzzle);
        
        // Initially puzzle should not be complete
        REQUIRE_FALSE(puzzle_system->is_puzzle_complete("test_puzzle"));
        
        // Update puzzle system (should check conditions)
        puzzle_system->update(0.016f);
        
        // Since agents are already in target positions, puzzle should be complete
        REQUIRE(puzzle_system->is_puzzle_complete("test_puzzle"));
        
        // Check completed puzzles list
        const auto& completed = puzzle_system->get_completed_puzzles();
        REQUIRE(completed.size() == 1);
        REQUIRE(completed[0] == "test_puzzle");
    }
    
    SECTION("Create position-based puzzle condition") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::pair<float, float>> target_positions = {{100.0f, 100.0f}, {200.0f, 200.0f}};
        
        auto condition = puzzle_system->create_position_condition("pos_test", agents, target_positions, 16.0f);
        
        REQUIRE(condition.condition_id == "pos_test");
        REQUIRE(condition.condition_type == "position");
        REQUIRE(condition.required_entities.size() == 2);
        REQUIRE(condition.parameters.at("tolerance") == 16.0f);
        REQUIRE(condition.parameters.at("target_x_0") == 100.0f);
        REQUIRE(condition.parameters.at("target_y_0") == 100.0f);
        REQUIRE(condition.parameters.at("target_x_1") == 200.0f);
        REQUIRE(condition.parameters.at("target_y_1") == 200.0f);
    }
    
    SECTION("Create inventory-based puzzle condition") {
        // Create test agents
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::string> required_items = {"key", "card"};
        
        auto condition = puzzle_system->create_inventory_condition("inv_test", agents, required_items);
        
        REQUIRE(condition.condition_id == "inv_test");
        REQUIRE(condition.condition_type == "inventory");
        REQUIRE(condition.required_entities.size() == 2);
        REQUIRE(condition.parameters.at("item_count") == 2.0f);
    }
    
    SECTION("Generate action feedback") {
        // Create test agents
        EntityID source_agent = entity_manager->create_entity();
        EntityID affected_agent = entity_manager->create_entity();
        
        std::vector<EntityID> affected_agents = {affected_agent};
        
        // Generate feedback
        puzzle_system->generate_action_feedback(source_agent, affected_agents, "movement");
        
        // Check that feedback was generated
        const auto& feedback = puzzle_system->get_active_feedback();
        REQUIRE(feedback.size() == 1);
        REQUIRE(feedback[0].source_agent == source_agent);
        REQUIRE(feedback[0].affected_agents.size() == 1);
        REQUIRE(feedback[0].affected_agents[0] == affected_agent);
        REQUIRE(feedback[0].feedback_type == "text");
    }
    
    SECTION("Update feedback display") {
        // Create test agents
        EntityID source_agent = entity_manager->create_entity();
        EntityID affected_agent = entity_manager->create_entity();
        
        std::vector<EntityID> affected_agents = {affected_agent};
        
        // Generate feedback with short duration
        puzzle_system->generate_action_feedback(source_agent, affected_agents, "movement");
        
        // Verify feedback was created
        REQUIRE_FALSE(puzzle_system->get_active_feedback().empty());
        
        // Modify feedback duration for testing
        auto& feedback = const_cast<std::vector<ActionFeedback>&>(puzzle_system->get_active_feedback());
        if (!feedback.empty()) {
            feedback[0].display_duration = 0.001f; // Very short duration
        }
        
        // Wait a bit to ensure time passes
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Update with enough time to expire feedback
        puzzle_system->update(0.1f); // 100ms should be enough to expire 1ms feedback
        
        // Feedback should be removed
        REQUIRE(puzzle_system->get_active_feedback().empty());
    }
}

TEST_CASE("PuzzleSystem condition checking", "[puzzle]") {
    // Create test dependencies
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    auto reality_manager = std::make_unique<RealityManager>();
    auto coordination_system = std::make_unique<CoordinationSystem>(reality_manager.get());
    
    // Create puzzle system
    auto puzzle_system = std::make_unique<PuzzleSystem>();
    
    // Initialize systems
    coordination_system->initialize(*entity_manager, *component_registry);
    puzzle_system->initialize(*entity_manager, *component_registry);
    puzzle_system->set_coordination_system(coordination_system.get());
    
    SECTION("Position condition with agents in correct positions") {
        // Create test agents at target positions
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f});
        component_registry->add_component<Transform>(agent2, Transform{200.0f, 200.0f, 0.0f, 1.0f, 1.0f});
        
        // Create puzzle with position condition
        Puzzle test_puzzle("pos_puzzle", "Position Test", true);
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::pair<float, float>> target_positions = {{100.0f, 100.0f}, {200.0f, 200.0f}};
        auto condition = puzzle_system->create_position_condition("pos_condition", agents, target_positions, 16.0f);
        test_puzzle.conditions.push_back(condition);
        
        puzzle_system->register_puzzle(test_puzzle);
        puzzle_system->update(0.016f);
        
        REQUIRE(puzzle_system->is_puzzle_complete("pos_puzzle"));
    }
    
    SECTION("Position condition with agents in wrong positions") {
        // Create test agents at wrong positions
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        component_registry->add_component<Transform>(agent1, Transform{50.0f, 50.0f, 0.0f, 1.0f, 1.0f});   // Wrong position
        component_registry->add_component<Transform>(agent2, Transform{150.0f, 150.0f, 0.0f, 1.0f, 1.0f}); // Wrong position
        
        // Create puzzle with position condition
        Puzzle test_puzzle("pos_puzzle_fail", "Position Test Fail", true);
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::pair<float, float>> target_positions = {{100.0f, 100.0f}, {200.0f, 200.0f}};
        auto condition = puzzle_system->create_position_condition("pos_condition", agents, target_positions, 16.0f);
        test_puzzle.conditions.push_back(condition);
        
        puzzle_system->register_puzzle(test_puzzle);
        puzzle_system->update(0.016f);
        
        REQUIRE_FALSE(puzzle_system->is_puzzle_complete("pos_puzzle_fail"));
    }
    
    SECTION("Inventory condition with required items") {
        // Create test agents with inventories
        EntityID agent1 = entity_manager->create_entity();
        EntityID agent2 = entity_manager->create_entity();
        component_registry->add_component<Agent>(agent1, Agent{1, false, 100.0f});
        component_registry->add_component<Agent>(agent2, Agent{2, false, 100.0f});
        
        // Add inventories with enough items
        Inventory inv1;
        inv1.items.insert("item1");
        inv1.items.insert("item2");
        inv1.items.insert("item3");
        component_registry->add_component<Inventory>(agent1, inv1);
        
        Inventory inv2;
        inv2.items.insert("item1");
        inv2.items.insert("item2");
        component_registry->add_component<Inventory>(agent2, inv2);
        
        // Create puzzle with inventory condition
        Puzzle test_puzzle("inv_puzzle", "Inventory Test", true);
        std::vector<EntityID> agents = {agent1, agent2};
        std::vector<std::string> required_items = {"key", "card"}; // 2 items required
        auto condition = puzzle_system->create_inventory_condition("inv_condition", agents, required_items);
        test_puzzle.conditions.push_back(condition);
        
        puzzle_system->register_puzzle(test_puzzle);
        puzzle_system->update(0.016f);
        
        // Should be complete since both agents have >= 2 items
        REQUIRE(puzzle_system->is_puzzle_complete("inv_puzzle"));
    }
}