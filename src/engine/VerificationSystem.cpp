#include "VerificationSystem.h"
#include "RealitySystem.h"
#include "HUDSystem.h"
#include <iostream>
#include <sstream>

VerificationSystem::VerificationSystem(EntityManager* entity_manager, ComponentRegistry* component_registry)
    : entity_manager_(entity_manager), component_registry_(component_registry) {
    reset_scenario();
}

void VerificationSystem::set_conditions(const std::vector<LevelCondition>& conditions) {
    active_conditions_ = conditions;
    reset_scenario();
    if (!conditions.empty()) {
        std::ostringstream initial_feedback;
        initial_feedback << "Mission Objectives:";
        for (const auto& condition : conditions) {
            initial_feedback << "\n- " << get_condition_description(condition);
        }
        feedback_message_ = initial_feedback.str();
        if (hud_system_) {
            hud_system_->set_verification_message(feedback_message_, 8.0f);
        }
    }
}

void VerificationSystem::update(float delta_time) {
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    if (feedback_timer_ > 0.0f) {
        feedback_timer_ -= delta_time;
    }
    
    if (scenario_completed_ || scenario_failed_) {
        return;
    }
    
    if (!active_conditions_.empty()) {
        conditions_met_ = check_all_conditions();
        
        if (conditions_met_) {
            if (!scenario_completed_) {
                scenario_completed_ = true;
                feedback_message_ = "MISSION ACCOMPLISHED: All objectives completed successfully!";
                feedback_timer_ = 5.0f;
                std::cout << "Verification Scenario: " << feedback_message_ << std::endl;
                
                if (hud_system_) {
                    hud_system_->set_verification_message(feedback_message_, 5.0f);
                }
            }
        } else {
            update_feedback();
        }
    } else {
        agent_reached_success_zone_ = check_success_zone();
        agent_has_required_ability_ = check_required_ability();
        correct_reality_active_ = check_reality_state();
        
        if (agent_reached_success_zone_ && agent_has_required_ability_ && correct_reality_active_) {
            scenario_completed_ = true;
            feedback_message_ = "SUCCESS: Agent successfully reached the success zone with required abilities!";
            feedback_timer_ = 5.0f;
            std::cout << "Verification Scenario: " << feedback_message_ << std::endl;
            
            if (hud_system_) {
                hud_system_->set_verification_message(feedback_message_, 5.0f);
            }
        } else if (agent_reached_success_zone_ && (!agent_has_required_ability_ || !correct_reality_active_)) {
            scenario_failed_ = true;
            
            std::ostringstream detailed_error;
            detailed_error << "UNEXPECTED: Agent reached success zone but conditions not met. ";
            detailed_error << "Success Zone: YES, ";
            detailed_error << "Required Ability: " << (agent_has_required_ability_ ? "YES" : "NO");
            
            const auto* agent_container = component_registry_->get_all_components<Agent>();
            if (agent_container) {
                const auto& agent_entities = agent_container->get_entities();
                for (EntityID agent_entity : agent_entities) {
                    auto* agent = component_registry_->get_component<Agent>(agent_entity);
                    if (agent && agent->is_possessed) {
                        auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
                        auto* inventory = component_registry_->get_component<Inventory>(agent_entity);
                        
                        detailed_error << " (Agent has: ";
                        if (loadout && loadout->current_ability != AbilityType::None) {
                            detailed_error << "LoadoutAbility=" << static_cast<int>(loadout->current_ability);
                        }
                        if (inventory && !inventory->abilities.empty()) {
                            detailed_error << " InventoryAbilities=";
                            for (const auto& ability : inventory->abilities) {
                                detailed_error << ability.first << ":" << ability.second << " ";
                            }
                        }
                        detailed_error << ")";
                        break;
                    }
                }
            }
            
            detailed_error << ", Correct Reality: " << (correct_reality_active_ ? "YES" : "NO");
            
            feedback_message_ = detailed_error.str();
            feedback_timer_ = 5.0f;
            std::cout << "Verification Scenario: " << feedback_message_ << std::endl;
            
            if (hud_system_) {
                hud_system_->set_verification_message(feedback_message_, 5.0f);
            }
        } else {
            update_feedback();
        }
    }
}

bool VerificationSystem::check_success_zone() {
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return false;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    
    for (EntityID agent_entity : agent_entities) {
        auto* agent = component_registry_->get_component<Agent>(agent_entity);
        auto* transform = component_registry_->get_component<Transform>(agent_entity);
        
        if (!agent || !transform) {
            continue;
        }
        
        if (!agent->is_possessed) {
            continue;
        }
        
        const auto* trigger_container = component_registry_->get_all_components<Trigger>();
        if (!trigger_container) {
            continue;
        }
        
        const auto& trigger_entities = trigger_container->get_entities();
        
        for (EntityID trigger_entity : trigger_entities) {
            auto* trigger = component_registry_->get_component<Trigger>(trigger_entity);
            auto* trigger_transform = component_registry_->get_component<Transform>(trigger_entity);
            
            if (!trigger || !trigger_transform) {
                continue;
            }
            
            if (trigger->trigger_type != "success_zone") {
                continue;
            }
            
            float agent_half_width = 16.0f; 
            float agent_half_height = 16.0f;
            
            float agent_left = transform->x - agent_half_width;
            float agent_right = transform->x + agent_half_width;
            float agent_top = transform->y - agent_half_height;
            float agent_bottom = transform->y + agent_half_height;
            
            float trigger_half_width = trigger->width * 0.5f;
            float trigger_half_height = trigger->height * 0.5f;
            
            float trigger_left = trigger_transform->x - trigger_half_width;
            float trigger_right = trigger_transform->x + trigger_half_width;
            float trigger_top = trigger_transform->y - trigger_half_height;
            float trigger_bottom = trigger_transform->y + trigger_half_height;
            
            if (agent_right > trigger_left && agent_left < trigger_right &&
                agent_bottom > trigger_top && agent_top < trigger_bottom) {
                
                trigger->has_been_triggered = true;
                trigger->triggering_entity = agent_entity;
                
                return true;
            }
        }
    }
    
    return false;
}

bool VerificationSystem::check_required_ability() {
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return false;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    
    for (EntityID agent_entity : agent_entities) {
        auto* agent = component_registry_->get_component<Agent>(agent_entity);
        auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
        auto* inventory = component_registry_->get_component<Inventory>(agent_entity);
        
        if (!agent) {
            continue;
        }
        
        if (!agent->is_possessed) {
            continue;
        }
        
        if (loadout && loadout->current_ability == AbilityType::DoubleJump) {
            return true;
        }
        
        if (inventory && (inventory->abilities.count("phase_shift") > 0 || inventory->items.count("phase_shift") > 0)) {
            return true;
        }
        
        if (loadout && loadout->current_ability != AbilityType::None) {
            return true;
        }
    }
    
    return false;
}

bool VerificationSystem::check_reality_state() {
    if (reality_manager_) {
        return reality_manager_->get_current_reality() == Reality::B;
    }
    
    return true; 
}

void VerificationSystem::update_feedback() {
    std::ostringstream feedback;
    
    if (!active_conditions_.empty()) {
        bool first = true;
        for (const auto& condition : active_conditions_) {
            if (!check_condition(condition)) {
                if (!first) feedback << "\n";
                if (condition.type == "agent_has_ability") {
                     feedback << "Need " << condition.value << " ability (Find Quantum Node)";
                } else if (condition.type == "trigger_activated" && condition.target == "success_zone") {
                     feedback << "Go to Success Zone (Green Area)";
                } else if (condition.type == "agent_position") {
                     feedback << "Get Agent " << condition.target << " to target position";
                } else {
                     feedback << "Objective incomplete: " << get_condition_description(condition);
                }
                first = false;
            }
        }
        
        if (first) {
             feedback << "All objectives complete! Mission Accomplished.";
        }
    } else {
        if (!agent_has_required_ability_) {
            feedback << "Interact with quantum nodes to gain abilities needed for the puzzle.";
        } else if (!agent_reached_success_zone_) {
            feedback << "Navigate to the success zone (green area) to complete the level.";
        } else {
            feedback << "Verification in progress...";
        }
    }
    
    if (feedback_timer_ <= 0.0f && feedback.str() != feedback_message_) {
        feedback_message_ = feedback.str();
        feedback_timer_ = 2.0f;
        
        if (hud_system_) {
            hud_system_->set_verification_message(feedback_message_, 2.0f);
        }
    }
}

bool VerificationSystem::check_all_conditions() {
    for (const auto& condition : active_conditions_) {
        if (!check_condition(condition)) {
            return false;
        }
    }
    return true;
}

bool VerificationSystem::check_condition(const LevelCondition& condition) {
    if (condition.type == "agent_position") {
        try {
            int agent_num = std::stoi(condition.target);
            
            const auto* agent_container = component_registry_->get_all_components<Agent>();
            if (!agent_container) return false;
            
            const auto& agent_entities = agent_container->get_entities();
            for (EntityID entity : agent_entities) {
                auto* agent = component_registry_->get_component<Agent>(entity);
                if (agent && agent->agent_number == agent_num) {
                    auto* transform = component_registry_->get_component<Transform>(entity);
                    if (!transform) return false;
                    
                    size_t gt_pos = condition.value.find('>');
                    size_t lt_pos = condition.value.find('<');
                    
                    if (gt_pos != std::string::npos) {
                        std::string axis = condition.value.substr(0, gt_pos);
                        float val = std::stof(condition.value.substr(gt_pos + 1));
                        if (axis == "x") return transform->x > val;
                        if (axis == "y") return transform->y > val;
                    } else if (lt_pos != std::string::npos) {
                        std::string axis = condition.value.substr(0, lt_pos);
                        float val = std::stof(condition.value.substr(lt_pos + 1));
                        if (axis == "x") return transform->x < val;
                        if (axis == "y") return transform->y < val;
                    }
                    return false;
                }
            }
        } catch (...) {
            return false;
        }
    } else if (condition.type == "agent_has_ability") {
        try {
            int agent_num = std::stoi(condition.target);
            std::string ability_name = condition.value;
            
            AbilityType required_ability = AbilityType::None;
            if (ability_name == "double_jump") required_ability = AbilityType::DoubleJump;
            else if (ability_name == "dash") required_ability = AbilityType::Dash;
            else if (ability_name == "axe") required_ability = AbilityType::Axe;
            else if (ability_name == "keycard") required_ability = AbilityType::Keycard;
            else if (ability_name == "water_walk") required_ability = AbilityType::WaterWalk;
            else if (ability_name == "phase_shift") required_ability = AbilityType::PhaseShift;
            
             const auto* agent_container = component_registry_->get_all_components<Agent>();
            if (!agent_container) return false;
            
            const auto& agent_entities = agent_container->get_entities();
            for (EntityID entity : agent_entities) {
                auto* agent = component_registry_->get_component<Agent>(entity);
                if (agent && agent->agent_number == agent_num) {
                    // Check active loadout first
                    auto* loadout = component_registry_->get_component<LoadoutComponent>(entity);
                    if (loadout && loadout->current_ability == required_ability && required_ability != AbilityType::None) {
                        return true;
                    }
                    
                    auto* inventory = component_registry_->get_component<Inventory>(entity);
                    if (inventory) {
                        if (inventory->abilities.count(ability_name) > 0 || inventory->items.count(ability_name) > 0) {
                            return true;
                        }
                    }

                    return false;
                }
            }
        } catch (...) {
            return false;
        }
    } else if (condition.type == "trigger_activated") {
        std::string trigger_type = condition.target;
        bool expected_state = (condition.value == "true");
        
        const auto* trigger_container = component_registry_->get_all_components<Trigger>();
        if (!trigger_container) return false;
        
        const auto& trigger_entities = trigger_container->get_entities();
        for (EntityID entity : trigger_entities) {
            auto* trigger = component_registry_->get_component<Trigger>(entity);
            if (trigger && trigger->trigger_type == trigger_type) {
                if (trigger_type == "success_zone") {
                    bool is_triggered = false;
                    
                    const auto* agent_container = component_registry_->get_all_components<Agent>();
                    if (agent_container) {
                         const auto& agent_entities = agent_container->get_entities();
                         for (EntityID agent_id : agent_entities) {
                             auto* agent = component_registry_->get_component<Agent>(agent_id);
                             if (agent && agent->is_possessed) {
                                  auto* agent_transform = component_registry_->get_component<Transform>(agent_id);
                                  auto* trigger_transform = component_registry_->get_component<Transform>(entity);
                                  
                                  if (agent_transform && trigger_transform) {
                                      float agent_half_w = 16.0f;
                                      float agent_half_h = 16.0f;
                                      float trigger_half_w = trigger->width * 0.5f;
                                      float trigger_half_h = trigger->height * 0.5f;
                                      
                                      if (std::abs(agent_transform->x - trigger_transform->x) < (agent_half_w + trigger_half_w) &&
                                          std::abs(agent_transform->y - trigger_transform->y) < (agent_half_h + trigger_half_h)) {
                                          is_triggered = true;
                                          break;
                                      }
                                  }
                             }
                         }
                    }
                    
                    if (is_triggered == expected_state) {
                         return true;
                    }
                } else {
                    if (trigger->has_been_triggered == expected_state) {
                        return true;
                    }
                }
            }
        }
        return false;
    } else if (condition.type == "reality_state") {
        if (!reality_manager_) return false;
        
        Reality current = reality_manager_->get_current_reality();
        if (condition.value == "A") return current == Reality::A;
        if (condition.value == "B") return current == Reality::B;
    }
    
    return false;
}

std::string VerificationSystem::get_condition_description(const LevelCondition& condition) {
    std::ostringstream desc;
    if (condition.type == "agent_position") {
        desc << "Agent " << condition.target << " must reach " << condition.value;
    } else if (condition.type == "agent_has_ability") {
        desc << "Agent " << condition.target << " must have " << condition.value << " ability";
    } else if (condition.type == "trigger_activated") {
        desc << "Activate " << condition.target;
    } else {
        desc << condition.type << ": " << condition.target << " = " << condition.value;
    }
    return desc.str();
}

void VerificationSystem::reset_scenario() {
    scenario_completed_ = false;
    scenario_failed_ = false;
    feedback_message_ = "Verification Scenario: Interact with quantum nodes to gain abilities, then reach the success zone.";
    feedback_timer_ = 3.0f;
    
    agent_reached_success_zone_ = false;
    agent_has_required_ability_ = false;
    correct_reality_active_ = false;
    
    std::cout << "Verification Scenario Reset" << std::endl;
}

std::string VerificationSystem::get_verification_stats() const {
    std::ostringstream stats;
    stats << "Verification Stats:\n";
    stats << "  Success Zone: " << (agent_reached_success_zone_ ? "YES" : "NO") << "\n";
    stats << "  Required Ability: " << (agent_has_required_ability_ ? "YES" : "NO") << "\n";
    stats << "  Correct Reality: " << (correct_reality_active_ ? "YES" : "NO") << "\n";
    stats << "  Completed: " << (scenario_completed_ ? "YES" : "NO") << "\n";
    stats << "  Failed: " << (scenario_failed_ ? "YES" : "NO") << "\n";
    stats << "  Feedback: " << feedback_message_;
    
    return stats.str();
}

void VerificationSystem::set_hud_system(HUDSystem* hud_system) {
    hud_system_ = hud_system;
}
void VerificationSystem::set_reality_manager(RealityManager* reality_manager) {
    reality_manager_ = reality_manager;
}

void VerificationSystem::reset() {
    reset_scenario();
    active_conditions_.clear();
    std::cout << "Verification System reset" << std::endl;
}
