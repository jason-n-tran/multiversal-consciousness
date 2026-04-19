#include "MovementSystem.h"
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
    std::cout << "MovementSystem shutdown" << std::endl;
}

void MovementSystem::apply_movement(EntityID entity, float delta_time) {
    if (!component_registry_) {
        return;
    }
    
    Agent* agent = component_registry_->get_component<Agent>(entity);
    Transform* transform = component_registry_->get_component<Transform>(entity);
    
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
    if (input_manager_->is_action_active(InputAction::MOVE_UP)) {
        move_y -= 1.0f;
    }
    if (input_manager_->is_action_active(InputAction::MOVE_DOWN)) {
        move_y += 1.0f;
    }
    
    if (move_x != 0.0f && move_y != 0.0f) {
        float length = std::sqrt(move_x * move_x + move_y * move_y);
        move_x /= length;
        move_y /= length;
    }
    
    float movement_distance = agent->movement_speed * delta_time;
    transform->x += move_x * movement_distance;
    transform->y += move_y * movement_distance;
    
    if (move_x != 0.0f || move_y != 0.0f) {
        std::cout << "Moving agent " << static_cast<int>(agent->agent_number) 
                  << " to (" << transform->x << ", " << transform->y << ")" << std::endl;
    }
}

void MovementSystem::set_input_manager(InputManager* input_manager) {
    input_manager_ = input_manager;
}

void MovementSystem::set_possession_system(PossessionSystem* possession_system) {
    possession_system_ = possession_system;
}