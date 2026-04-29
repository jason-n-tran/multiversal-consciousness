#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "InputManager.h"
#include "PossessionSystem.h"
#include "InteractiveObstacles.h"
#include <memory>
#include <unordered_map>

class InteractiveObstacleSystem : public ISystem {
private:
    InputManager* input_manager_{nullptr};           
    PossessionSystem* possession_system_{nullptr};  
    
    std::unordered_map<EntityID, std::unique_ptr<IInteractable>> obstacles_;
    
    EntityID nearby_interactable_{INVALID_ENTITY};
    
    void update_proximity_detection(EntityID possessed_agent);
    
    bool is_within_interaction_range(EntityID agent_entity, EntityID obstacle_entity) const;
    
    void handle_interaction_input(EntityID possessed_agent);
    
    std::unique_ptr<IInteractable> create_obstacle(EntityID entity, InteractionType type);
    
    void update_interaction_prompts(EntityID possessed_agent);
    
public:
    InteractiveObstacleSystem() = default;
    
    ~InteractiveObstacleSystem() override = default;
    
    InteractiveObstacleSystem(const InteractiveObstacleSystem&) = delete;
    InteractiveObstacleSystem& operator=(const InteractiveObstacleSystem&) = delete;
    
    InteractiveObstacleSystem(InteractiveObstacleSystem&&) noexcept = default;
    InteractiveObstacleSystem& operator=(InteractiveObstacleSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    void set_input_manager(InputManager* input_manager);
    
    void set_possession_system(PossessionSystem* possession_system);
    
    void register_obstacle(EntityID entity);
    
    void unregister_obstacle(EntityID entity);
    
    EntityID get_nearby_interactable() const;
    
    bool force_interaction(EntityID agent_entity, EntityID obstacle_entity);
};