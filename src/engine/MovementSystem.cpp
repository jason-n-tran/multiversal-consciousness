#include "MovementSystem.h"
#include "QuantumLoadoutSystem.h"
#include <iostream>
#include <cmath>

void MovementSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    std::cout << "MovementSystem initialized" << std::endl;
}

void MovementSystem::update(float delta_time) {
    if (!input_manager_ || !possession_system_) {
        return;
    }
    
    auto possessed_entity = possession_system_->get_possessed_entity();
    if (!possessed_entity.has_value()) {
        return;
    }
    
    apply_movement(possessed_entity.value(), delta_time);
}

void MovementSystem::shutdown() {
    input_manager_ = nullptr;
    possession_system_ = nullptr;
    loadout_system_ = nullptr;
    std::cout << "MovementSystem shutdown" << std::endl;
}

void MovementSystem::apply_movement(EntityID entity, float delta_time) {
    if (!component_registry_) {
        return;
    }
    
    Agent* agent = component_registry_->get_component<Agent>(entity);
    Transform* transform = component_registry_->get_component<Transform>(entity);
    PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
    
    if (!agent || !transform) {
        return; 
    }
    
    float move_x = 0.0f;
    float move_y = 0.0f;
    
    if (input_manager_->is_action_active(InputAction::MOVE_LEFT)) {
        move_x -= 1.0f;
    }
    if (input_manager_->is_action_active(InputAction::MOVE_RIGHT)) {
        move_x += 1.0f;
    }
    bool jump_pressed = input_manager_->is_action_just_pressed(InputAction::JUMP);
    bool jumped = handle_jumping(entity, physics, agent, jump_pressed);
    if (input_manager_->is_action_active(InputAction::MOVE_DOWN)) {
        move_y += 1.0f;
    }
    
    if (move_x != 0.0f && move_y != 0.0f) {
        float length = std::sqrt(move_x * move_x + move_y * move_y);
        move_x /= length;
        move_y /= length;
    }
    
    if (physics) {
        float movement_force = agent->movement_speed;
        
        if (move_x != 0.0f) {
            physics->velocity_x = move_x * movement_force;
        } else {
            physics->velocity_x *= 0.8f; 
        }
        
        if (move_y > 0.0f) {
            physics->velocity_y = move_y * movement_force;
        }
        
        if (move_x != 0.0f || jumped) {
            std::cout << "Physics movement for agent " << static_cast<int>(agent->agent_number) 
                      << " velocity: (" << physics->velocity_x << ", " << physics->velocity_y << ")" << std::endl;
        }
    } else {
        float movement_distance = agent->movement_speed * delta_time;
        transform->x += move_x * movement_distance;
        transform->y += move_y * movement_distance;
        
        if (move_x != 0.0f || move_y != 0.0f) {
            std::cout << "Direct movement for agent " << static_cast<int>(agent->agent_number) 
                      << " to (" << transform->x << ", " << transform->y << ")" << std::endl;
        }
    }
}

void MovementSystem::set_input_manager(InputManager* input_manager) {
    input_manager_ = input_manager;
}

void MovementSystem::set_possession_system(PossessionSystem* possession_system) {
    possession_system_ = possession_system;
}

void MovementSystem::set_loadout_system(QuantumLoadoutSystem* loadout_system) {
    loadout_system_ = loadout_system;
}

bool MovementSystem::handle_jumping(EntityID entity, PhysicsComponent* physics, Agent* agent, bool jump_pressed) {
    if (!physics || !agent || !jump_pressed) {
        return false;
    }
    
    bool can_jump = physics->is_grounded;
    bool is_double_jump = false;
    
    if (!can_jump && loadout_system_ && !physics->has_double_jumped) {
        AbilityType current_ability = loadout_system_->get_current_ability(entity);
        if (current_ability == AbilityType::DoubleJump) {
            can_jump = !physics->is_grounded && loadout_system_->can_use_current_ability(entity);
            is_double_jump = true;
            
            if (can_jump) {
                loadout_system_->use_ability(entity);
                physics->has_double_jumped = true;
                std::cout << "Agent " << static_cast<int>(agent->agent_number) << " used DoubleJump!" << std::endl;
            }
        }
    }
    
    if (can_jump) {
        float movement_force = agent->movement_speed;
        physics->velocity_y = -movement_force * 2.0f; 
        
        if (physics->is_grounded) {
            physics->is_grounded = false;
            physics->has_double_jumped = false; 
        }
        
        std::cout << "Agent " << static_cast<int>(agent->agent_number) << " jumped!" << std::endl;
        return true;
    }
    
    return false;
}