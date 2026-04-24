#include "QuantumSystem.h"
#include "PossessionSystem.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <iostream>

QuantumSystem::QuantumSystem(std::unique_ptr<RealityManager> reality_manager)
    : reality_manager_(std::move(reality_manager)) {
    if (!reality_manager_) {
        throw std::invalid_argument("RealityManager cannot be null");
    }
}

void QuantumSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    entity_manager_ = &entity_manager;
    component_registry_ = &component_registry;
}

void QuantumSystem::update(float delta_time) {
    if (input_manager_ && input_manager_->is_action_just_pressed(InputAction::INTERACT)) {
        // Check if possessed agent is near a quantum node
        if (possession_system_) {
            auto possessed_entity = possession_system_->get_possessed_entity();
            if (possessed_entity.has_value()) {
                // Find nearby quantum nodes and trigger interaction
                trigger_interaction_for_agent(possessed_entity.value());
            }
        }
    }
    
    update_interaction_prompts(delta_time);
    
    while (!pending_interactions_.empty()) {
        const QuantumInteraction& interaction = pending_interactions_.front();
        
        if (entity_manager_->is_valid(interaction.node_entity) &&
            entity_manager_->is_valid(interaction.agent_entity) &&
            validate_entity_components(interaction.node_entity, false) &&
            validate_entity_components(interaction.agent_entity, true)) {
            
            if (is_agent_in_range(interaction.node_entity, interaction.agent_entity)) {
                process_quantum_distribution(interaction);
            }
        }
        
        pending_interactions_.pop();
    }
}

void QuantumSystem::shutdown() {
    while (!pending_interactions_.empty()) {
        pending_interactions_.pop();
    }
    
    agent_prompt_targets_.clear();
}

void QuantumSystem::update_interaction_prompts(float delta_time) {
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    
    std::unordered_map<EntityID, EntityID> current_nearby;
    
    for (EntityID agent_entity : agent_entities) {
        if (!entity_manager_->is_valid(agent_entity)) {
            continue;
        }
        
        std::vector<EntityID> nearby_nodes = get_nearby_quantum_nodes(agent_entity);
        
        if (!nearby_nodes.empty()) {
            EntityID nearest_node = nearby_nodes[0];
            current_nearby[agent_entity] = nearest_node;
            
            auto it = agent_prompt_targets_.find(agent_entity);
            if (it == agent_prompt_targets_.end() || it->second != nearest_node) {
                show_interaction_prompt(agent_entity, nearest_node);
                agent_prompt_targets_[agent_entity] = nearest_node;
            }
        } else {
            auto it = agent_prompt_targets_.find(agent_entity);
            if (it != agent_prompt_targets_.end()) {
                hide_interaction_prompt(agent_entity);
                agent_prompt_targets_.erase(it);
            }
        }
    }
    
    const auto* prompt_container = component_registry_->get_all_components<InteractionPrompt>();
    if (prompt_container) {
        const auto& prompt_entities = prompt_container->get_entities();
        for (EntityID prompt_entity : prompt_entities) {
            InteractionPrompt* prompt = component_registry_->get_component<InteractionPrompt>(prompt_entity);
            if (prompt && prompt->is_visible) {
                prompt->display_duration += delta_time;
            }
        }
    }
}

void QuantumSystem::show_interaction_prompt(EntityID agent_entity, EntityID node_entity) {
    InteractionPrompt* existing_prompt = component_registry_->get_component<InteractionPrompt>(agent_entity);
    
    if (existing_prompt) {
        existing_prompt->target_entity = node_entity;
        existing_prompt->is_visible = true;
        existing_prompt->display_duration = 0.0f;
        existing_prompt->prompt_text = "Press E to interact with Quantum Node";
    } else {
        InteractionPrompt new_prompt;
        new_prompt.target_entity = node_entity;
        new_prompt.is_visible = true;
        new_prompt.display_duration = 0.0f;
        new_prompt.prompt_text = "Press E to interact with Quantum Node";
        
        component_registry_->add_component<InteractionPrompt>(agent_entity, new_prompt);
    }
}

void QuantumSystem::hide_interaction_prompt(EntityID agent_entity) {
    InteractionPrompt* prompt = component_registry_->get_component<InteractionPrompt>(agent_entity);
    if (prompt) {
        prompt->is_visible = false;
        prompt->display_duration = 0.0f;
    }
}

bool QuantumSystem::trigger_quantum_node(EntityID node_entity, EntityID agent_entity) {
    if (!entity_manager_->is_valid(node_entity) || !entity_manager_->is_valid(agent_entity)) {
        return false;
    }
    
    if (!validate_entity_components(node_entity, false) || !validate_entity_components(agent_entity, true)) {
        return false;
    }
    
    if (!is_agent_in_range(node_entity, agent_entity)) {
        return false;
    }
    
    const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(node_entity);
    if (quantum_node && quantum_node->is_activated) {
    }
    Reality current_reality = reality_manager_->get_current_reality();
    pending_interactions_.emplace(node_entity, agent_entity, current_reality);
    
    return true;
}

bool QuantumSystem::is_agent_in_range(EntityID node_entity, EntityID agent_entity) const {
    const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(node_entity);
    if (!quantum_node) {
        return false;
    }
    float distance = calculate_distance(node_entity, agent_entity);
    if (distance < 0.0f) {
    }
    
    return distance <= quantum_node->interaction_radius;
}

std::vector<EntityID> QuantumSystem::get_nearby_quantum_nodes(EntityID agent_entity) const {
    std::vector<EntityID> nearby_nodes;
    
    if (!entity_manager_->is_valid(agent_entity) || !validate_entity_components(agent_entity, true)) {
        return nearby_nodes;
    }
    
    const auto* quantum_container = component_registry_->get_all_components<QuantumNode>();
    if (!quantum_container) {
        return nearby_nodes;
    }
    
    const auto& quantum_entities = quantum_container->get_entities();
    
    for (EntityID node_entity : quantum_entities) {
        if (is_agent_in_range(node_entity, agent_entity)) {
            nearby_nodes.push_back(node_entity);
        }
    }
    
    return nearby_nodes;
}

void QuantumSystem::process_quantum_distribution(const QuantumInteraction& interaction) {
    const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(interaction.node_entity);
    if (!quantum_node) {
        return;
    }
    
    add_item_to_agent(interaction.agent_entity, quantum_node->reality_a_item, Reality::A);
    add_item_to_agent(interaction.agent_entity, quantum_node->reality_b_item, Reality::B);
    
    activate_quantum_node(interaction.node_entity);
}

float QuantumSystem::calculate_distance(EntityID entity1, EntityID entity2) const {
    const Transform* transform1 = component_registry_->get_component<Transform>(entity1);
    const Transform* transform2 = component_registry_->get_component<Transform>(entity2);
    
    if (!transform1 || !transform2) {
        return -1.0f;
    }
    
    float dx = transform1->x - transform2->x;
    float dy = transform1->y - transform2->y;
    return std::sqrt(dx * dx + dy * dy);
}

bool QuantumSystem::validate_entity_components(EntityID entity, bool require_agent) const {
    if (!component_registry_->has_component<Transform>(entity)) {
        return false;
    }
    
    if (require_agent) {
        return component_registry_->has_component<Agent>(entity);
    } else {
        return component_registry_->has_component<QuantumNode>(entity);
    }
}

void QuantumSystem::add_item_to_agent(EntityID agent_entity, const std::string& item_name, Reality reality) {
    if (item_name.empty()) {
        return;
    }
    
    const Inventory* current_inventory = reality_manager_->get_reality_inventory(agent_entity, reality);
    
    Inventory updated_inventory;
    if (current_inventory) {
        updated_inventory = *current_inventory;
    }
    
    updated_inventory.items.insert(item_name);
    
    reality_manager_->set_reality_inventory(agent_entity, updated_inventory, reality);
}

void QuantumSystem::activate_quantum_node(EntityID node_entity) {
    QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(node_entity);
    if (!quantum_node) {
        return;
    }
    
    quantum_node->is_activated = true;
    
    reality_manager_->set_reality_quantum_node(node_entity, *quantum_node, Reality::A);
    reality_manager_->set_reality_quantum_node(node_entity, *quantum_node, Reality::B);
}