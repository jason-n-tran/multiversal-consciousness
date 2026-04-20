#include "CoordinationSystem.h"
#include <algorithm>
#include <cmath>

CoordinationSystem::CoordinationSystem(std::unique_ptr<RealityManager> reality_manager)
    : reality_manager_(std::move(reality_manager)) {
    recent_actions_.reserve(MAX_ACTION_HISTORY);
    pending_effects_.reserve(50); 
}

void CoordinationSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);
    
    initialize_effect_evaluators();
}

void CoordinationSystem::update(float delta_time) {
    evaluate_inter_agent_effects();
    
    apply_pending_effects();
    
    update_timing_mechanics(delta_time);
    
    cleanup_action_history();
}

void CoordinationSystem::shutdown() {
    recent_actions_.clear();
    pending_effects_.clear();
    active_mechanics_.clear();
    puzzle_participants_.clear();
    puzzle_completion_status_.clear();
    effect_evaluators_.clear();
}

void CoordinationSystem::record_agent_action(EntityID agent_entity, const std::string& action_type, 
                                            Reality reality, const std::unordered_map<std::string, float>& parameters) {
    AgentAction action(agent_entity, action_type, reality);
    action.parameters = parameters;
    
    recent_actions_.push_back(action);
    
    if (recent_actions_.size() > MAX_ACTION_HISTORY) {
        recent_actions_.erase(recent_actions_.begin());
    }
}

void CoordinationSystem::evaluate_inter_agent_effects() {
    for (const auto& action : recent_actions_) {
        auto evaluator_it = effect_evaluators_.find(action.action_type);
        if (evaluator_it != effect_evaluators_.end()) {
            evaluator_it->second(action);
        }
    }
}

void CoordinationSystem::apply_pending_effects() {
    for (const auto& effect : pending_effects_) {
        if (effect.effect_type == "position_change") {
            if (auto* transform = component_registry_->get_component<Transform>(effect.target_agent)) {
                auto dx_it = effect.effect_data.find("dx");
                auto dy_it = effect.effect_data.find("dy");
                if (dx_it != effect.effect_data.end() && dy_it != effect.effect_data.end()) {
                    transform->x += dx_it->second;
                    transform->y += dy_it->second;
                }
            }
        }
        else if (effect.effect_type == "item_received") {
            if (auto* inventory = component_registry_->get_component<Inventory>(effect.target_agent)) {
                auto item_it = effect.effect_data.find("item_id");
                if (item_it != effect.effect_data.end()) {
                    std::string item_name = "item_" + std::to_string(static_cast<int>(item_it->second));
                    inventory->items.insert(item_name);
                }
            }
        }
        else if (effect.effect_type == "environmental_change") {
            auto entity_it = effect.effect_data.find("entity_id");
            if (entity_it != effect.effect_data.end()) {
                EntityID env_entity = static_cast<EntityID>(entity_it->second);
                
                if (auto* door = component_registry_->get_component<Door>(env_entity)) {
                    auto state_it = effect.effect_data.find("new_state");
                    if (state_it != effect.effect_data.end()) {
                        door->is_open = (state_it->second > 0.5f);
                        reality_manager_->sync_shared_door(env_entity, *door);
                    }
                }
                else if (auto* water = component_registry_->get_component<WaterLevel>(env_entity)) {
                    auto level_it = effect.effect_data.find("new_level");
                    if (level_it != effect.effect_data.end()) {
                        water->target_level = level_it->second;
                        reality_manager_->sync_shared_water_level(env_entity, *water);
                    }
                }
            }
        }
    }
    
    pending_effects_.clear();
}

void CoordinationSystem::register_puzzle_scenario(const std::string& puzzle_id, const std::vector<EntityID>& participating_agents) {
    puzzle_participants_[puzzle_id] = participating_agents;
    puzzle_completion_status_[puzzle_id] = false;
}

bool CoordinationSystem::is_puzzle_complete(const std::string& puzzle_id) const {
    auto it = puzzle_completion_status_.find(puzzle_id);
    return it != puzzle_completion_status_.end() && it->second;
}

bool CoordinationSystem::start_timing_mechanic(const std::string& mechanic_id,
                                              const std::vector<EntityID>& required_agents,
                                              std::chrono::milliseconds time_window,
                                              std::function<bool()> completion_check) {
    if (active_mechanics_.find(mechanic_id) != active_mechanics_.end()) {
        return false; 
    }
    
    TimingSensitiveMechanic mechanic(mechanic_id, required_agents, time_window, completion_check);
    mechanic.is_active = true;
    mechanic.start_time = std::chrono::steady_clock::now();
    
    active_mechanics_[mechanic_id] = std::move(mechanic);
    return true;
}

void CoordinationSystem::update_timing_mechanics(float /*delta_time*/) {
    auto current_time = std::chrono::steady_clock::now();
    
    for (auto it = active_mechanics_.begin(); it != active_mechanics_.end();) {
        auto& mechanic = it->second;
        
        if (!mechanic.is_active) {
            ++it;
            continue;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - mechanic.start_time);
        if (elapsed > mechanic.time_window) {
            mechanic.is_active = false;
            ++it;
            continue;
        }
        
        if (mechanic.completion_check && mechanic.completion_check()) {
            mechanic.is_active = false;
            
            auto puzzle_it = puzzle_participants_.find(mechanic.mechanic_id);
            if (puzzle_it != puzzle_participants_.end()) {
                puzzle_completion_status_[mechanic.mechanic_id] = true;
            }
        }
        
        ++it;
    }
    
    for (auto it = active_mechanics_.begin(); it != active_mechanics_.end();) {
        if (!it->second.is_active) {
            auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(current_time - it->second.start_time);
            if (elapsed > std::chrono::minutes(5)) { 
                it = active_mechanics_.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void CoordinationSystem::handle_reality_switch() {
    for (auto& [mechanic_id, mechanic] : active_mechanics_) {
        if (mechanic.is_active) {
            mechanic.participating_agents.clear();
            
            for (EntityID agent : mechanic.required_agents) {
                if (entity_manager_->is_valid(agent)) {
                    mechanic.participating_agents.insert(agent);
                }
            }
        }
    }
}

void CoordinationSystem::initialize_effect_evaluators() {
    effect_evaluators_["movement"] = [this](const AgentAction& action) {
        evaluate_movement_effects(action);
    };
    
    effect_evaluators_["activation"] = [this](const AgentAction& action) {
        evaluate_activation_effects(action);
    };
    
    effect_evaluators_["quantum_interaction"] = [this](const AgentAction& action) {
        evaluate_quantum_effects(action);
    };
}

void CoordinationSystem::evaluate_movement_effects(const AgentAction& action) {
    auto* agent_transform = component_registry_->get_component<Transform>(action.agent_entity);
    if (!agent_transform) return;
    
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (agent_container) {
        const auto& agent_entities = agent_container->get_entities();
        for (EntityID other_agent : agent_entities) {
            if (other_agent == action.agent_entity) continue; 
            
            if (are_agents_in_range(action.agent_entity, other_agent, 50.0f)) {
                AgentEffect effect(action.agent_entity, other_agent, "proximity_effect", action.action_reality);
                effect.effect_data["distance"] = calculate_distance(action.agent_entity, other_agent);
                pending_effects_.push_back(effect);
            }
        }
    }
    
    const auto* door_container = component_registry_->get_all_components<Door>();
    if (door_container) {
        const auto& door_entities = door_container->get_entities();
        for (EntityID door_entity : door_entities) {
            if (calculate_distance(action.agent_entity, door_entity) < 32.0f) {
                auto* agent_inventory = component_registry_->get_component<Inventory>(action.agent_entity);
                auto* door = component_registry_->get_component<Door>(door_entity);
                
                if (agent_inventory && door && !door->required_key.empty()) {
                    if (agent_inventory->items.count(door->required_key) > 0) {
                        AgentEffect effect(action.agent_entity, action.agent_entity, "environmental_change", action.action_reality);
                        effect.effect_data["entity_id"] = static_cast<float>(door_entity);
                        effect.effect_data["new_state"] = 1.0f;
                        pending_effects_.push_back(effect);
                    }
                }
            }
        }
    }
}

void CoordinationSystem::evaluate_activation_effects(const AgentAction& action) {
    const auto* switch_container = component_registry_->get_all_components<EnvironmentalSwitch>();
    if (switch_container) {
        const auto& switch_entities = switch_container->get_entities();
        for (EntityID switch_entity : switch_entities) {
            if (calculate_distance(action.agent_entity, switch_entity) < 32.0f) {
                auto* env_switch = component_registry_->get_component<EnvironmentalSwitch>(switch_entity);
                if (env_switch && !env_switch->is_activated) {
                    // Activate the switch
                    env_switch->is_activated = true;
                    
                    if (env_switch->target_entity_type == "door") {
                        AgentEffect effect(action.agent_entity, action.agent_entity, "environmental_change", action.action_reality);
                        effect.effect_data["entity_id"] = std::stof(env_switch->target_entity_id);
                        effect.effect_data["new_state"] = 1.0f;
                        pending_effects_.push_back(effect);
                    }
                    else if (env_switch->target_entity_type == "water") {
                        AgentEffect effect(action.agent_entity, action.agent_entity, "environmental_change", action.action_reality);
                        effect.effect_data["entity_id"] = std::stof(env_switch->target_entity_id);
                        effect.effect_data["new_level"] = 0.0f;
                        pending_effects_.push_back(effect);
                    }
                }
            }
        }
    }
}

void CoordinationSystem::evaluate_quantum_effects(const AgentAction& action) {
    const auto* quantum_container = component_registry_->get_all_components<QuantumNode>();
    if (quantum_container) {
        const auto& quantum_entities = quantum_container->get_entities();
        for (EntityID quantum_entity : quantum_entities) {
            if (calculate_distance(action.agent_entity, quantum_entity) < 32.0f) {
                auto* quantum_node = component_registry_->get_component<QuantumNode>(quantum_entity);
                if (quantum_node && !quantum_node->is_activated) {
                    const auto* agent_container = component_registry_->get_all_components<Agent>();
                    if (agent_container) {
                        const auto& agent_entities = agent_container->get_entities();
                        for (EntityID target_agent : agent_entities) {
                            AgentEffect effect_a(action.agent_entity, target_agent, "item_received", Reality::A);
                            effect_a.effect_data["item_id"] = 1.0f;
                            pending_effects_.push_back(effect_a);
                            
                            AgentEffect effect_b(action.agent_entity, target_agent, "item_received", Reality::B);
                            effect_b.effect_data["item_id"] = 2.0f; 
                            pending_effects_.push_back(effect_b);
                        }
                    }
                    
                    quantum_node->is_activated = true;
                }
            }
        }
    }
}

void CoordinationSystem::cleanup_action_history() {
    auto current_time = std::chrono::steady_clock::now();
    auto cutoff_time = current_time - std::chrono::seconds(10);
    
    recent_actions_.erase(
        std::remove_if(recent_actions_.begin(), recent_actions_.end(),
            [cutoff_time](const AgentAction& action) {
                return action.timestamp < cutoff_time;
            }),
        recent_actions_.end()
    );
}

bool CoordinationSystem::are_agents_in_range(EntityID agent1, EntityID agent2, float max_distance) const {
    float distance = calculate_distance(agent1, agent2);
    return distance >= 0.0f && distance <= max_distance;
}

float CoordinationSystem::calculate_distance(EntityID entity1, EntityID entity2) const {
    auto* transform1 = component_registry_->get_component<Transform>(entity1);
    auto* transform2 = component_registry_->get_component<Transform>(entity2);
    
    if (!transform1 || !transform2) {
        return -1.0f; 
    }
    
    float dx = transform2->x - transform1->x;
    float dy = transform2->y - transform1->y;
    return std::sqrt(dx * dx + dy * dy);
}