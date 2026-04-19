#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "InputManager.h"
#include "PossessionSystem.h"
#include <memory>

class MovementSystem : public ISystem {
private:
    InputManager* input_manager_{nullptr};
    PossessionSystem* possession_system_{nullptr};
    
    void apply_movement(EntityID entity, float delta_time);
    
public:
    MovementSystem() = default;
    
    ~MovementSystem() override = default;
    
    MovementSystem(const MovementSystem&) = delete;
    MovementSystem& operator=(const MovementSystem&) = delete;
    
    MovementSystem(MovementSystem&&) noexcept = default;
    MovementSystem& operator=(MovementSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    void set_input_manager(InputManager* input_manager);
    
    void set_possession_system(PossessionSystem* possession_system);
};