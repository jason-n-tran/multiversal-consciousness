#include <catch2/catch_test_macros.hpp>
#include "engine/QuantumSystem.h"
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/RealityManager.h"
#include "engine/Components.h"
#include <memory>

TEST_CASE("QuantumSystem basic functionality", "[quantum]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    auto reality_manager = std::make_unique<RealityManager>();
    QuantumSystem quantum_system(reality_manager.get());
    
    quantum_system.initialize(entity_manager, component_registry);
    
    SECTION("System initializes correctly") {
        REQUIRE(quantum_system.get_pending_interaction_count() == 0);
        REQUIRE(quantum_system.get_reality_manager().get_current_reality() == Reality::A);
    }
    
    SECTION("Distance calculation works correctly") {
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        Transform transform1{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Transform transform2{3.0f, 4.0f, 0.0f, 1.0f, 1.0f};
        
        component_registry.add_component<Transform>(entity1, transform1);
        component_registry.add_component<Transform>(entity2, transform2);
        
        // Distance should be 5.0 (3-4-5 triangle)
        // Note: We can't directly test calculate_distance as it's private,
        // but we can test it through is_agent_in_range
        
        QuantumNode quantum_node;
        quantum_node.interaction_radius = 6.0f; // Should be in range
        component_registry.add_component<QuantumNode>(entity1, quantum_node);
        
        Agent agent;
        component_registry.add_component<Agent>(entity2, agent);
        
        REQUIRE(quantum_system.is_agent_in_range(entity1, entity2) == true);
        
        // Test with smaller radius
        quantum_node.interaction_radius = 4.0f; // Should be out of range
        component_registry.add_component<QuantumNode>(entity1, quantum_node);
        
        REQUIRE(quantum_system.is_agent_in_range(entity1, entity2) == false);
    }
}

TEST_CASE("QuantumSystem proximity detection", "[quantum]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    auto reality_manager = std::make_unique<RealityManager>();
    QuantumSystem quantum_system(reality_manager.get());
    
    quantum_system.initialize(entity_manager, component_registry);
    
    SECTION("Agent in range detection") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node
        Transform node_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        node_component.reality_a_item = "keycard";
        node_component.reality_b_item = "emp_device";
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent
        Transform agent_transform{120.0f, 120.0f, 0.0f, 1.0f, 1.0f}; // Distance ~28.28
        Agent agent_component;
        agent_component.agent_number = 1;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Agent should be in range
        REQUIRE(quantum_system.is_agent_in_range(quantum_node, agent) == true);
        
        // Move agent out of range
        agent_transform.x = 200.0f;
        agent_transform.y = 200.0f; // Distance ~141.42
        component_registry.add_component<Transform>(agent, agent_transform);
        
        REQUIRE(quantum_system.is_agent_in_range(quantum_node, agent) == false);
    }
    
    SECTION("Get nearby quantum nodes") {
        EntityID agent = entity_manager.create_entity();
        EntityID node1 = entity_manager.create_entity();
        EntityID node2 = entity_manager.create_entity();
        EntityID node3 = entity_manager.create_entity();
        
        // Set up agent
        Transform agent_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Set up quantum nodes
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        
        // Node 1 - in range
        Transform node1_transform{30.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        component_registry.add_component<Transform>(node1, node1_transform);
        component_registry.add_component<QuantumNode>(node1, node_component);
        
        // Node 2 - in range
        Transform node2_transform{0.0f, 40.0f, 0.0f, 1.0f, 1.0f};
        component_registry.add_component<Transform>(node2, node2_transform);
        component_registry.add_component<QuantumNode>(node2, node_component);
        
        // Node 3 - out of range
        Transform node3_transform{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
        component_registry.add_component<Transform>(node3, node3_transform);
        component_registry.add_component<QuantumNode>(node3, node_component);
        
        std::vector<EntityID> nearby_nodes = quantum_system.get_nearby_quantum_nodes(agent);
        
        REQUIRE(nearby_nodes.size() == 2);
        // The exact order might vary, so check that both node1 and node2 are present
        bool found_node1 = std::find(nearby_nodes.begin(), nearby_nodes.end(), node1) != nearby_nodes.end();
        bool found_node2 = std::find(nearby_nodes.begin(), nearby_nodes.end(), node2) != nearby_nodes.end();
        bool found_node3 = std::find(nearby_nodes.begin(), nearby_nodes.end(), node3) != nearby_nodes.end();
        
        REQUIRE(found_node1 == true);
        REQUIRE(found_node2 == true);
        REQUIRE(found_node3 == false);
    }
}

TEST_CASE("QuantumSystem interaction triggering", "[quantum]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    auto reality_manager = std::make_unique<RealityManager>();
    QuantumSystem quantum_system(reality_manager.get());
    
    quantum_system.initialize(entity_manager, component_registry);
    
    SECTION("Successful interaction triggering") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        node_component.reality_a_item = "keycard";
        node_component.reality_b_item = "emp_device";
        node_component.is_activated = false;
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent in range
        Transform agent_transform{20.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Trigger interaction
        bool trigger_success = quantum_system.trigger_quantum_node(quantum_node, agent);
        REQUIRE(trigger_success == true);
        REQUIRE(quantum_system.get_pending_interaction_count() == 1);
    }
    
    SECTION("Failed interaction - agent out of range") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 30.0f;
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent out of range
        Transform agent_transform{100.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Trigger interaction should fail
        bool trigger_success = quantum_system.trigger_quantum_node(quantum_node, agent);
        REQUIRE(trigger_success == false);
        REQUIRE(quantum_system.get_pending_interaction_count() == 0);
    }
    
    SECTION("Failed interaction - already activated node") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node (already activated)
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        node_component.is_activated = true; // Already activated
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent in range
        Transform agent_transform{20.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Trigger interaction should fail
        bool trigger_success = quantum_system.trigger_quantum_node(quantum_node, agent);
        REQUIRE(trigger_success == false);
        REQUIRE(quantum_system.get_pending_interaction_count() == 0);
    }
}

TEST_CASE("QuantumSystem interaction prompts", "[quantum]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    auto reality_manager = std::make_unique<RealityManager>();
    QuantumSystem quantum_system(reality_manager.get());
    
    quantum_system.initialize(entity_manager, component_registry);
    
    SECTION("Interaction prompts appear when agent approaches") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent in range
        Transform agent_transform{20.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Update system to trigger prompt logic
        quantum_system.update(0.016f); // ~60 FPS
        
        // Check if prompt was created
        const InteractionPrompt* prompt = component_registry.get_component<InteractionPrompt>(agent);
        REQUIRE(prompt != nullptr);
        REQUIRE(prompt->is_visible == true);
        REQUIRE(prompt->target_entity == quantum_node);
        REQUIRE(prompt->prompt_text == "Press E to interact with Quantum Node");
    }
    
    SECTION("Interaction prompts disappear when agent moves away") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent in range initially
        Transform agent_transform{20.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Update to create prompt
        quantum_system.update(0.016f);
        
        // Verify prompt exists
        const InteractionPrompt* prompt = component_registry.get_component<InteractionPrompt>(agent);
        REQUIRE(prompt != nullptr);
        REQUIRE(prompt->is_visible == true);
        
        // Move agent out of range
        agent_transform.x = 100.0f;
        component_registry.add_component<Transform>(agent, agent_transform);
        
        // Update to hide prompt
        quantum_system.update(0.016f);
        
        // Verify prompt is hidden
        prompt = component_registry.get_component<InteractionPrompt>(agent);
        REQUIRE(prompt != nullptr);
        REQUIRE(prompt->is_visible == false);
    }
}

TEST_CASE("QuantumSystem complete interaction workflow", "[quantum]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    auto reality_manager = std::make_unique<RealityManager>();
    QuantumSystem quantum_system(reality_manager.get());
    
    quantum_system.initialize(entity_manager, component_registry);
    
    SECTION("Complete quantum node interaction workflow") {
        EntityID quantum_node = entity_manager.create_entity();
        EntityID agent = entity_manager.create_entity();
        
        // Set up quantum node with different items for each reality
        Transform node_transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        QuantumNode node_component;
        node_component.interaction_radius = 50.0f;
        node_component.reality_a_item = "keycard";
        node_component.reality_b_item = "emp_device";
        node_component.is_activated = false;
        
        component_registry.add_component<Transform>(quantum_node, node_transform);
        component_registry.add_component<QuantumNode>(quantum_node, node_component);
        
        // Set up agent in range
        Transform agent_transform{20.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        Agent agent_component;
        agent_component.agent_number = 1;
        
        component_registry.add_component<Transform>(agent, agent_transform);
        component_registry.add_component<Agent>(agent, agent_component);
        
        // Step 1: Agent approaches - prompt should appear
        quantum_system.update(0.016f);
        
        const InteractionPrompt* prompt = component_registry.get_component<InteractionPrompt>(agent);
        REQUIRE(prompt != nullptr);
        REQUIRE(prompt->is_visible == true);
        
        // Step 2: Agent interacts with quantum node
        bool interaction_success = quantum_system.trigger_quantum_node(quantum_node, agent);
        REQUIRE(interaction_success == true);
        REQUIRE(quantum_system.get_pending_interaction_count() == 1);
        
        // Step 3: Process the interaction
        quantum_system.update(0.016f);
        
        // Verify interaction was processed
        REQUIRE(quantum_system.get_pending_interaction_count() == 0);
        
        // Step 4: Verify quantum node is activated
        const QuantumNode* updated_node = component_registry.get_component<QuantumNode>(quantum_node);
        REQUIRE(updated_node != nullptr);
        REQUIRE(updated_node->is_activated == true);
        
        // Step 5: Verify items were distributed to both realities
        const Inventory* inventory_a = quantum_system.get_reality_manager().get_reality_inventory(agent, Reality::A);
        const Inventory* inventory_b = quantum_system.get_reality_manager().get_reality_inventory(agent, Reality::B);
        
        REQUIRE(inventory_a != nullptr);
        REQUIRE(inventory_b != nullptr);
        REQUIRE(inventory_a->items.count("keycard") == 1);
        REQUIRE(inventory_b->items.count("emp_device") == 1);
        
        // Step 6: Verify agent can't interact with the same node again
        bool second_interaction = quantum_system.trigger_quantum_node(quantum_node, agent);
        REQUIRE(second_interaction == false);
    }
}