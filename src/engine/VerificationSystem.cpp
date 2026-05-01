#include "VerificationSystem.h"
#include "RealitySystem.h"
#include <iostream>
#include <sstream>

VerificationSystem::VerificationSystem(EntityManager* entity_manager, ComponentRegistry* component_registry)
    : entity_manager_(entity_manager), component_registry_(component_registry) {
    reset_scenario();
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
    
    agent_reached_success_zone_ = check_success_zone();
    agent_has_required_ability_ = check_required_ability();
    correct_reality_active_ = check_reality_state();
    
    if (agent_reached_success_zone_ && agent_has_required_ability_ && correct_reality_active_) {
        scenario_completed_ = true;
        feedback_message_ = "SUCCESS: Agent successfully traversed the high wall using DoubleJump in Reality B!";
        feedback_timer_ = 5.0f;
        std::cout << "Verification Scenario: " << feedback_message_ << std::endl;
    } else if (agent_reached_success_zone_ && (!agent_has_required_ability_ || !correct_reality_active_)) {
        scenario_failed_ = true;
        feedback_message_ = "UNEXPECTED: Agent reached success zone without proper conditions. Check physics system.";
        feedback_timer_ = 5.0f;
        std::cout << "Verification Scenario: " << feedback_message_ << std::endl;
    } else {
        update_feedback();
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
        
        if (!agent || !loadout) {
            continue;
        }
        
        if (!agent->is_possessed) {
            continue;
        }
        
        return loadout->current_ability == AbilityType::DoubleJump;
    }
    
    return false;
}

bool VerificationSystem::check_reality_state() {
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return false;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    
    for (EntityID agent_entity : agent_entities) {
        auto* agent = component_registry_->get_component<Agent>(agent_entity);
        auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
        
        if (!agent || !loadout) {
            continue;
        }
        
        if (!agent->is_possessed) {
            continue;
        }
        
        if (loadout->current_ability == AbilityType::DoubleJump) {
            return true;
        }
    }
    
    return false;
}

void VerificationSystem::update_feedback() {
    std::ostringstream feedback;
    
    if (!correct_reality_active_ && !agent_has_required_ability_) {
        feedback << "Reality A active. Interact with quantum node to switch to Reality B and gain DoubleJump.";
    } else if (correct_reality_active_ && agent_has_required_ability_) {
        feedback << "Reality B active with DoubleJump ability. Try jumping over the high wall!";
    } else if (!agent_reached_success_zone_) {
        feedback << "Navigate to the success zone on the right side of the wall.";
    } else {
        feedback << "Verification in progress...";
    }
    
    if (feedback_timer_ <= 0.0f && feedback.str() != feedback_message_) {
        feedback_message_ = feedback.str();
        feedback_timer_ = 2.0f;
    }
}

void VerificationSystem::reset_scenario() {
    scenario_completed_ = false;
    scenario_failed_ = false;
    feedback_message_ = "Verification Scenario: Navigate to quantum node, switch to Reality B, and traverse the high wall.";
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