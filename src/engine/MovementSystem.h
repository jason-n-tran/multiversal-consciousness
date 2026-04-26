#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "InputManager.h"
#include "PossessionSystem.h"
#include <memory>

class QuantumLoadoutSystem;

class MovementSystem : public ISystem {
private:
    InputManager* input_manager_{nullptr};
    PossessionSystem* possession_system_{nullptr};
    QuantumLoadoutSystem* loadout_system_{nullptr};
    
    void apply_movement(EntityID entity, float delta_time);

    bool handle_jumping(EntityID entity, PhysicsComponent* physics, Agent* agent, bool jump_pressed);
    
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

    void set_loadout_system(QuantumLoadoutSystem* loadout_system);
};