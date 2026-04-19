#include "PossessionSystem.h"
#include <iostream>

void PossessionSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    std::cout << "PossessionSystem initialized" << std::endl;
    
    camera_ = std::make_unique<CameraController>();
    camera_->initialize(component_registry, 800, 600);
    
    update_agent_mappings();
}

void PossessionSystem::update(float delta_time) {
    update_agent_mappings();
    
    if (camera_) {
        camera_->update(delta_time);
    }

    if (input_manager_) {
        if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_1)) {
            possess_agent(1);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_2)) {
            possess_agent(2);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_3)) {
            possess_agent(3);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_4)) {
            possess_agent(4);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_5)) {
            possess_agent(5);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_6)) {
            possess_agent(6);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_7)) {
            possess_agent(7);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_8)) {
            possess_agent(8);
        } else if (input_manager_->is_action_just_pressed(InputAction::POSSESS_AGENT_9)) {
            possess_agent(9);
        }
    } else {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                handle_input(event);
            }
        }
    }
}

void PossessionSystem::shutdown() {
    release_possession();
    
    agent_mappings_.clear();
    
    camera_.reset();
    
    std::cout << "PossessionSystem shutdown" << std::endl;
}

void PossessionSystem::update_agent_mappings() {
    if (!component_registry_) {
        return;
    }
    
    agent_mappings_.clear();
    
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& entities = agent_container->get_entities();
    const auto& agents = agent_container->get_components();
    
    for (size_t i = 0; i < entities.size(); ++i) {
        const EntityID entity = entities[i];
        const Agent& agent = agents[i];
        
        if (agent.agent_number >= 1 && agent.agent_number <= 9) {
            agent_mappings_[agent.agent_number] = entity;
        }
    }
}

void PossessionSystem::set_agent_possession_state(EntityID entity, bool is_possessed) {
    if (!component_registry_) {
        return;
    }
    
    Agent* agent = component_registry_->get_component<Agent>(entity);
    if (agent) {
        agent->is_possessed = is_possessed;
    }
}

bool PossessionSystem::possess_agent(uint8_t agent_number) {
    if (agent_number < 1 || agent_number > 9) {
        return false;
    }
    
    auto it = agent_mappings_.find(agent_number);
    if (it == agent_mappings_.end()) {
        return false;
    }
    
    EntityID target_entity = it->second;
    
    if (possessed_entity_.has_value()) {
        set_agent_possession_state(possessed_entity_.value(), false);
    }
    
    possessed_entity_ = target_entity;
    set_agent_possession_state(target_entity, true);
    
    update_camera_target();
    
    std::cout << "Possessed agent " << static_cast<int>(agent_number) 
              << " (Entity ID: " << target_entity << ")" << std::endl;
    
    return true;
}

void PossessionSystem::release_possession() {
    if (possessed_entity_.has_value()) {
        set_agent_possession_state(possessed_entity_.value(), false);
        
        std::cout << "Released possession of Entity ID: " << possessed_entity_.value() << std::endl;
        
        possessed_entity_.reset();
        
        if (camera_) {
            camera_->set_target_entity(std::nullopt);
        }
    }
}

std::optional<EntityID> PossessionSystem::get_possessed_entity() const {
    return possessed_entity_;
}

bool PossessionSystem::is_entity_possessed(EntityID entity) const {
    return possessed_entity_.has_value() && possessed_entity_.value() == entity;
}

uint8_t PossessionSystem::get_agent_number(EntityID entity) const {
    if (!component_registry_) {
        return 0;
    }
    
    const Agent* agent = component_registry_->get_component<Agent>(entity);
    if (agent) {
        return agent->agent_number;
    }
    
    return 0; 
}

const std::unordered_map<uint8_t, EntityID>& PossessionSystem::get_agent_mappings() const {
    return agent_mappings_;
}

bool PossessionSystem::handle_input(const SDL_Event& event) {
    if (event.type != SDL_EVENT_KEY_DOWN) {
        return false;
    }
    
    switch (event.key.key) {
        case SDLK_1:
            return possess_agent(1);
        case SDLK_2:
            return possess_agent(2);
        case SDLK_3:
            return possess_agent(3);
        case SDLK_4:
            return possess_agent(4);
        case SDLK_5:
            return possess_agent(5);
        case SDLK_6:
            return possess_agent(6);
        case SDLK_7:
            return possess_agent(7);
        case SDLK_8:
            return possess_agent(8);
        case SDLK_9:
            return possess_agent(9);
        default:
            return false;
    }
}
void PossessionSystem::update_camera_target() {
    if (camera_ && possessed_entity_.has_value()) {
        camera_->set_target_entity(possessed_entity_.value());
    }
}

CameraController& PossessionSystem::get_camera_controller() {
    if (!camera_) {
        throw std::runtime_error("Camera controller not initialized");
    }
    return *camera_;
}

const CameraController& PossessionSystem::get_camera_controller() const {
    if (!camera_) {
        throw std::runtime_error("Camera controller not initialized");
    }
    return *camera_;
}

void PossessionSystem::set_camera_bounds(const CameraBounds& bounds) {
    if (camera_) {
        camera_->set_bounds(bounds);
    }
}
void PossessionSystem::set_agent_renderer(AgentRenderer* agent_renderer) {
    agent_renderer_ = agent_renderer;
    
    if (agent_renderer_ && camera_) {
        agent_renderer_->set_camera_controller(camera_.get());
    }
}

void PossessionSystem::set_input_manager(InputManager* input_manager) {
    input_manager_ = input_manager;
}