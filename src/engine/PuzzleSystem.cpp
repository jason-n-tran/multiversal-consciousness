#include "PuzzleSystem.h"
#include <algorithm>
#include <cmath>

void PuzzleSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    entity_manager_ = &entity_manager;
    component_registry_ = &component_registry;
    
    initialize_condition_checkers();
    initialize_feedback_generators();
}

void PuzzleSystem::update(float delta_time) {
    check_puzzle_completion();
    
    if (coordination_system_) {
        const auto& pending_effects = coordination_system_->get_pending_effects();
        for (const auto& effect : pending_effects) {
            auto generator_it = feedback_generators_.find(effect.effect_type);
            if (generator_it != feedback_generators_.end()) {
                generator_it->second(effect);
            }
        }
    }
    
    update_feedback_display(delta_time);
}

void PuzzleSystem::shutdown() {
    active_puzzles_.clear();
    completed_puzzles_.clear();
    active_feedback_.clear();
    feedback_generators_.clear();
    condition_checkers_.clear();
}

void PuzzleSystem::set_coordination_system(CoordinationSystem* coordination_system) {
    coordination_system_ = coordination_system;
}

void PuzzleSystem::register_puzzle(const Puzzle& puzzle) {
    active_puzzles_[puzzle.puzzle_id] = puzzle;
}

bool PuzzleSystem::add_puzzle_condition(const std::string& puzzle_id, const PuzzleCondition& condition) {
    auto puzzle_it = active_puzzles_.find(puzzle_id);
    if (puzzle_it == active_puzzles_.end()) {
        return false;
    }
    
    puzzle_it->second.conditions.push_back(condition);
    return true;
}

void PuzzleSystem::check_puzzle_completion() {
    for (auto& [puzzle_id, puzzle] : active_puzzles_) {
        if (puzzle.is_complete) {
            continue;
        }
        
        bool all_conditions_met = true;
        bool any_condition_met = false;
        
        for (auto& condition : puzzle.conditions) {
            auto checker_it = condition_checkers_.find(condition.condition_type);
            if (checker_it != condition_checkers_.end()) {
                condition.is_met = checker_it->second(condition);
            } else if (condition.check_function) {
                condition.is_met = condition.check_function();
            }
            
            if (condition.is_met) {
                any_condition_met = true;
            } else {
                all_conditions_met = false;
            }
        }
        
        bool is_complete = puzzle.require_all_conditions ? all_conditions_met : any_condition_met;
        
        if (is_complete && !puzzle.is_complete) {
            puzzle.is_complete = true;
            puzzle.completion_time = std::chrono::steady_clock::now();
            completed_puzzles_.push_back(puzzle_id);
            
            std::vector<EntityID> all_agents;
            for (const auto& condition : puzzle.conditions) {
                all_agents.insert(all_agents.end(), condition.required_entities.begin(), condition.required_entities.end());
            }
            
            std::sort(all_agents.begin(), all_agents.end());
            all_agents.erase(std::unique(all_agents.begin(), all_agents.end()), all_agents.end());
            
            if (!all_agents.empty()) {
                ActionFeedback completion_feedback(all_agents[0], all_agents, "puzzle_completion", 
                                                 "Puzzle '" + puzzle.puzzle_name + "' completed!");
                completion_feedback.display_duration = 5.0f; // Display longer for puzzle completion
                active_feedback_.push_back(completion_feedback);
            }
        }
    }
}

bool PuzzleSystem::is_puzzle_complete(const std::string& puzzle_id) const {
    auto puzzle_it = active_puzzles_.find(puzzle_id);
    return puzzle_it != active_puzzles_.end() && puzzle_it->second.is_complete;
}

void PuzzleSystem::generate_action_feedback(EntityID source_agent, const std::vector<EntityID>& affected_agents, 
                                           const std::string& action_type) {
    std::string feedback_message;
    
    if (action_type == "movement") {
        if (affected_agents.size() == 1) {
            feedback_message = "Agent movement affected nearby agent";
        } else if (affected_agents.size() > 1) {
            feedback_message = "Agent movement affected " + std::to_string(affected_agents.size()) + " nearby agents";
        }
    }
    else if (action_type == "activation") {
        feedback_message = "Agent activation affected environmental systems";
    }
    else if (action_type == "quantum_interaction") {
        feedback_message = "Quantum interaction distributed items across realities";
    }
    else {
        feedback_message = "Agent action had effects on other agents";
    }
    
    if (!feedback_message.empty()) {
        ActionFeedback feedback(source_agent, affected_agents, "text", feedback_message);
        active_feedback_.push_back(feedback);
    }
}

void PuzzleSystem::update_feedback_display(float /*delta_time*/) {
    auto current_time = std::chrono::steady_clock::now();
    
    for (auto it = active_feedback_.begin(); it != active_feedback_.end();) {
        auto& feedback = *it;
        
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - feedback.timestamp);
        
        if (elapsed.count() < feedback.display_duration) {
            feedback.is_displayed = true;
            ++it;
        } else {
            // Expired, remove feedback
            it = active_feedback_.erase(it);
        }
    }
}

PuzzleCondition PuzzleSystem::create_position_condition(const std::string& condition_id,
                                                       const std::vector<EntityID>& agents,
                                                       const std::vector<std::pair<float, float>>& target_positions,
                                                       float tolerance) {
    PuzzleCondition condition(condition_id, "position", agents, nullptr);
    
    for (size_t i = 0; i < target_positions.size() && i < agents.size(); ++i) {
        condition.parameters["target_x_" + std::to_string(i)] = target_positions[i].first;
        condition.parameters["target_y_" + std::to_string(i)] = target_positions[i].second;
    }
    condition.parameters["tolerance"] = tolerance;
    
    return condition;
}

PuzzleCondition PuzzleSystem::create_inventory_condition(const std::string& condition_id,
                                                        const std::vector<EntityID>& agents,
                                                        const std::vector<std::string>& required_items) {
    PuzzleCondition condition(condition_id, "inventory", agents, nullptr);
    
    for (size_t i = 0; i < required_items.size(); ++i) {
        condition.parameters["item_" + std::to_string(i)] = static_cast<float>(std::hash<std::string>{}(required_items[i]));
    }
    condition.parameters["item_count"] = static_cast<float>(required_items.size());
    
    return condition;
}

PuzzleCondition PuzzleSystem::create_environmental_condition(const std::string& condition_id,
                                                            const std::vector<EntityID>& env_entities,
                                                            const std::vector<std::string>& required_states) {
    PuzzleCondition condition(condition_id, "environmental", env_entities, nullptr);
    
    for (size_t i = 0; i < required_states.size() && i < env_entities.size(); ++i) {
        condition.parameters["state_" + std::to_string(i)] = static_cast<float>(std::hash<std::string>{}(required_states[i]));
    }
    
    return condition;
}

PuzzleCondition PuzzleSystem::create_timing_condition(const std::string& condition_id,
                                                     const std::vector<EntityID>& agents,
                                                     std::chrono::milliseconds time_window,
                                                     const std::string& action_type) {
    PuzzleCondition condition(condition_id, "timing", agents, nullptr);
    
    condition.parameters["time_window_ms"] = static_cast<float>(time_window.count());
    condition.parameters["action_type_hash"] = static_cast<float>(std::hash<std::string>{}(action_type));
    
    return condition;
}

void PuzzleSystem::initialize_condition_checkers() {
    condition_checkers_["position"] = [this](const PuzzleCondition& condition) {
        return check_position_condition(condition);
    };
    
    condition_checkers_["inventory"] = [this](const PuzzleCondition& condition) {
        return check_inventory_condition(condition);
    };
    
    condition_checkers_["environmental"] = [this](const PuzzleCondition& condition) {
        return check_environmental_condition(condition);
    };
    
    condition_checkers_["timing"] = [this](const PuzzleCondition& condition) {
        return check_timing_condition(condition);
    };
}

void PuzzleSystem::initialize_feedback_generators() {
    feedback_generators_["proximity_effect"] = [this](const AgentEffect& effect) {
        generate_movement_feedback(effect);
    };
    
    feedback_generators_["environmental_change"] = [this](const AgentEffect& effect) {
        generate_environmental_feedback(effect);
    };
    
    feedback_generators_["item_received"] = [this](const AgentEffect& effect) {
        generate_item_feedback(effect);
    };
}

bool PuzzleSystem::check_position_condition(const PuzzleCondition& condition) {
    float tolerance = condition.parameters.at("tolerance");
    
    for (size_t i = 0; i < condition.required_entities.size(); ++i) {
        EntityID agent = condition.required_entities[i];
        auto* transform = component_registry_->get_component<Transform>(agent);
        if (!transform) {
            return false; 
        }
        
        auto target_x_key = "target_x_" + std::to_string(i);
        auto target_y_key = "target_y_" + std::to_string(i);
        
        auto target_x_it = condition.parameters.find(target_x_key);
        auto target_y_it = condition.parameters.find(target_y_key);
        
        if (target_x_it == condition.parameters.end() || target_y_it == condition.parameters.end()) {
            continue;
        }
        
        float target_x = target_x_it->second;
        float target_y = target_y_it->second;
        
        float dx = transform->x - target_x;
        float dy = transform->y - target_y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > tolerance) {
            return false; 
        }
    }
    
    return true;
}

bool PuzzleSystem::check_inventory_condition(const PuzzleCondition& condition) {
    auto item_count_it = condition.parameters.find("item_count");
    if (item_count_it == condition.parameters.end()) {
        return false;
    }
    
    size_t required_item_count = static_cast<size_t>(item_count_it->second);
    
    for (EntityID agent : condition.required_entities) {
        auto* inventory = component_registry_->get_component<Inventory>(agent);
        if (!inventory) {
            return false; 
        }
        
        if (inventory->items.size() < required_item_count) {
            return false;
        }
    }
    
    return true;
}

bool PuzzleSystem::check_environmental_condition(const PuzzleCondition& condition) {
    for (size_t i = 0; i < condition.required_entities.size(); ++i) {
        EntityID env_entity = condition.required_entities[i];
        
        if (auto* door = component_registry_->get_component<Door>(env_entity)) {
            if (!door->is_open) {
                return false;
            }
        }
        else if (auto* water = component_registry_->get_component<WaterLevel>(env_entity)) {
            if (water->current_level != water->target_level) {
                return false;
            }
        }
        else if (auto* env_switch = component_registry_->get_component<EnvironmentalSwitch>(env_entity)) {
            if (!env_switch->is_activated) {
                return false;
            }
        }
    }
    
    return true;
}

bool PuzzleSystem::check_timing_condition(const PuzzleCondition& condition) {
    
    if (!coordination_system_) {
        return false;
    }
    
    auto time_window_it = condition.parameters.find("time_window_ms");
    if (time_window_it == condition.parameters.end()) {
        return false;
    }
    
    std::chrono::milliseconds time_window(static_cast<long>(time_window_it->second));
    
    const auto& active_mechanics = coordination_system_->get_active_mechanics();
    for (const auto& [mechanic_id, mechanic] : active_mechanics) {
        if (mechanic.is_active && mechanic.time_window == time_window) {
            bool all_participating = true;
            for (EntityID agent : condition.required_entities) {
                if (mechanic.participating_agents.find(agent) == mechanic.participating_agents.end()) {
                    all_participating = false;
                    break;
                }
            }
            
            if (all_participating) {
                return true;
            }
        }
    }
    
    return false;
}

void PuzzleSystem::generate_movement_feedback(const AgentEffect& effect) {
    std::vector<EntityID> affected = {effect.target_agent};
    ActionFeedback feedback(effect.source_agent, affected, "visual", 
                           "Agent movement created proximity effect");
    active_feedback_.push_back(feedback);
}

void PuzzleSystem::generate_environmental_feedback(const AgentEffect& effect) {
    std::vector<EntityID> affected = {effect.target_agent};
    ActionFeedback feedback(effect.source_agent, affected, "visual", 
                           "Environmental change affected the area");
    active_feedback_.push_back(feedback);
}

void PuzzleSystem::generate_item_feedback(const AgentEffect& effect) {
    std::vector<EntityID> affected = {effect.target_agent};
    ActionFeedback feedback(effect.source_agent, affected, "text", 
                           "Item received from quantum interaction");
    active_feedback_.push_back(feedback);
}

float PuzzleSystem::calculate_distance(EntityID entity1, EntityID entity2) const {
    auto* transform1 = component_registry_->get_component<Transform>(entity1);
    auto* transform2 = component_registry_->get_component<Transform>(entity2);
    
    if (!transform1 || !transform2) {
        return -1.0f;
    }
    
    float dx = transform2->x - transform1->x;
    float dy = transform2->y - transform1->y;
    return std::sqrt(dx * dx + dy * dy);
}