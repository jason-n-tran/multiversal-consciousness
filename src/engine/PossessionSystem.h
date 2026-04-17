#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "CameraController.h"
#include "AgentRenderer.h"
#include <SDL3/SDL.h>
#include <optional>
#include <unordered_map>
#include <memory>
#include <cstdint>


class PossessionSystem : public ISystem {
private:
    std::optional<EntityID> possessed_entity_;                  
    std::unordered_map<uint8_t, EntityID> agent_mappings_;      
    std::unique_ptr<CameraController> camera_;                 
    AgentRenderer* agent_renderer_{nullptr};                   
    
    void update_agent_mappings();
    
    void set_agent_possession_state(EntityID entity, bool is_possessed);
    
    void update_camera_target();
    
public:
    PossessionSystem() = default;
    
    ~PossessionSystem() override = default;
    
    PossessionSystem(const PossessionSystem&) = delete;
    PossessionSystem& operator=(const PossessionSystem&) = delete;
    
    PossessionSystem(PossessionSystem&&) noexcept = default;
    PossessionSystem& operator=(PossessionSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    bool possess_agent(uint8_t agent_number);
    
    void release_possession();
    
    std::optional<EntityID> get_possessed_entity() const;
    
    bool is_entity_possessed(EntityID entity) const;
    
    uint8_t get_agent_number(EntityID entity) const;
    
    const std::unordered_map<uint8_t, EntityID>& get_agent_mappings() const;
    
    CameraController& get_camera_controller();
    
    const CameraController& get_camera_controller() const;
    
    void set_camera_bounds(const CameraBounds& bounds);
    
    void set_agent_renderer(AgentRenderer* agent_renderer);
    
    bool handle_input(const SDL_Event& event);
};