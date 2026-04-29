#include "InteractiveObstacleSystem.h"
#include <cmath>
#include <limits>

void InteractiveObstacleSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);
    
    const auto* interactable_container = component_registry.get_all_components<InteractableComponent>();
    if (interactable_container) {
        const auto& entities = interactable_container->get_entities();
        for (EntityID entity : entities) {
            register_obstacle(entity);
        }
    }
}

void InteractiveObstacleSystem::update(float delta_time) {
    (void)delta_time; 
    
    if (!possession_system_ || !input_manager_) {
        return;
    }
    
    auto possessed_agent_opt = possession_system_->get_possessed_entity();
    if (!possessed_agent_opt.has_value()) {
        return;
    }
    
    EntityID possessed_agent = possessed_agent_opt.value();
    
    update_proximity_detection(possessed_agent);
    
    update_interaction_prompts(possessed_agent);
    
    handle_interaction_input(possessed_agent);
}

void InteractiveObstacleSystem::shutdown() {
    obstacles_.clear();
    nearby_interactable_ = INVALID_ENTITY;
}

void InteractiveObstacleSystem::set_input_manager(InputManager* input_manager) {
    input_manager_ = input_manager;
}

void InteractiveObstacleSystem::set_possession_system(PossessionSystem* possession_system) {
    possession_system_ = possession_system;
}

void InteractiveObstacleSystem::register_obstacle(EntityID entity) {
    if (!entity_manager_->is_valid(entity)) {
        return;
    }
    
    const auto* interactable = component_registry_->get_component<InteractableComponent>(entity);
    if (!interactable) {
        return;
    }
    
    auto obstacle = create_obstacle(entity, interactable->type);
    if (obstacle) {
        obstacles_[entity] = std::move(obstacle);
    }
}

void InteractiveObstacleSystem::unregister_obstacle(EntityID entity) {
    obstacles_.erase(entity);
    
    if (nearby_interactable_ == entity) {
        nearby_interactable_ = INVALID_ENTITY;
    }
}

EntityID InteractiveObstacleSystem::get_nearby_interactable() const {
    return nearby_interactable_;
}

bool InteractiveObstacleSystem::force_interaction(EntityID agent_entity, EntityID obstacle_entity) {
    if (!entity_manager_->is_valid(agent_entity) || !entity_manager_->is_valid(obstacle_entity)) {
        return false;
    }
    
    auto obstacle_it = obstacles_.find(obstacle_entity);
    if (obstacle_it == obstacles_.end()) {
        return false;
    }
    
    const auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    if (!loadout) {
        return false;
    }
    
    if (!obstacle_it->second->can_interact(agent_entity, *loadout)) {
        return false;
    }
    
    obstacle_it->second->interact(agent_entity, *entity_manager_, *component_registry_);
    
    if (!entity_manager_->is_valid(obstacle_entity)) {
        unregister_obstacle(obstacle_entity);
    }
    
    return true;
}

void InteractiveObstacleSystem::update_proximity_detection(EntityID possessed_agent) {
    const auto* agent_transform = component_registry_->get_component<Transform>(possessed_agent);
    if (!agent_transform) {
        nearby_interactable_ = INVALID_ENTITY;
        return;
    }
    
    EntityID closest_interactable = INVALID_ENTITY;
    float closest_distance = std::numeric_limits<float>::max();
    
    for (const auto& [entity, obstacle] : obstacles_) {
        if (!entity_manager_->is_valid(entity)) {
            continue;
        }
        
        const auto* interactable = component_registry_->get_component<InteractableComponent>(entity);
        if (!interactable || !interactable->is_active) {
            continue;
        }
        
        if (is_within_interaction_range(possessed_agent, entity)) {
            const auto* obstacle_transform = component_registry_->get_component<Transform>(entity);
            if (obstacle_transform) {
                float dx = agent_transform->x - obstacle_transform->x;
                float dy = agent_transform->y - obstacle_transform->y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance < closest_distance) {
                    closest_distance = distance;
                    closest_interactable = entity;
                }
            }
        }
    }
    
    nearby_interactable_ = closest_interactable;
}

bool InteractiveObstacleSystem::is_within_interaction_range(EntityID agent_entity, EntityID obstacle_entity) const {
    const auto* agent_transform = component_registry_->get_component<Transform>(agent_entity);
    const auto* obstacle_transform = component_registry_->get_component<Transform>(obstacle_entity);
    const auto* interactable = component_registry_->get_component<InteractableComponent>(obstacle_entity);
    
    if (!agent_transform || !obstacle_transform || !interactable) {
        return false;
    }
    
    float dx = agent_transform->x - obstacle_transform->x;
    float dy = agent_transform->y - obstacle_transform->y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance <= interactable->interaction_radius;
}

void InteractiveObstacleSystem::handle_interaction_input(EntityID possessed_agent) {
    if (nearby_interactable_ == INVALID_ENTITY) {
        return;
    }
    
    if (input_manager_->is_action_just_pressed(InputAction::INTERACT)) {
        force_interaction(possessed_agent, nearby_interactable_);
    }
}

std::unique_ptr<IInteractable> InteractiveObstacleSystem::create_obstacle(EntityID entity, InteractionType type) {
    switch (type) {
        case InteractionType::Tree:
            return std::make_unique<TreeObstacle>(entity);
        case InteractionType::Door:
            return std::make_unique<DoorObstacle>(entity);
        case InteractionType::Chasm:
            return std::make_unique<ChasmObstacle>(entity);
        case InteractionType::Switch:
            return nullptr;
        case InteractionType::QuantumNode:
            return nullptr;
        default:
            return nullptr;
    }
}

void InteractiveObstacleSystem::update_interaction_prompts(EntityID possessed_agent) {
    const auto* prompt_container = component_registry_->get_all_components<InteractionPrompt>();
    if (prompt_container) {
        const auto& entities = prompt_container->get_entities();
        for (EntityID entity : entities) {
            auto* prompt = component_registry_->get_component<InteractionPrompt>(entity);
            if (prompt) {
                prompt->is_visible = false;
            }
        }
    }
    
    if (nearby_interactable_ != INVALID_ENTITY) {
        const auto* interactable = component_registry_->get_component<InteractableComponent>(nearby_interactable_);
        const auto* loadout = component_registry_->get_component<LoadoutComponent>(possessed_agent);
        
        if (interactable && loadout) {
            auto obstacle_it = obstacles_.find(nearby_interactable_);
            if (obstacle_it != obstacles_.end()) {
                auto* prompt = component_registry_->get_component<InteractionPrompt>(nearby_interactable_);
                if (!prompt) {
                    InteractionPrompt new_prompt;
                    new_prompt.target_entity = nearby_interactable_;
                    new_prompt.is_visible = true;
                    new_prompt.prompt_text = obstacle_it->second->get_interaction_prompt();
                    component_registry_->add_component(nearby_interactable_, std::move(new_prompt));
                } else {
                    prompt->is_visible = true;
                    prompt->target_entity = nearby_interactable_;
                    prompt->prompt_text = obstacle_it->second->get_interaction_prompt();
                }
                
                if (!obstacle_it->second->can_interact(possessed_agent, *loadout)) {
                    auto* updated_prompt = component_registry_->get_component<InteractionPrompt>(nearby_interactable_);
                    if (updated_prompt) {
                        updated_prompt->prompt_text = obstacle_it->second->get_interaction_prompt() + " (Missing required ability)";
                    }
                }
            }
        }
    }
}