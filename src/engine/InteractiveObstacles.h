#pragma once

#include "IInteractable.h"
#include "Components.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include <string>

class TreeObstacle : public IInteractable {
private:
    EntityID entity_id_;
    
public:
    explicit TreeObstacle(EntityID entity_id) : entity_id_(entity_id) {}
    
    bool can_interact(EntityID agent_id, const LoadoutComponent& loadout) const override {
        (void)agent_id;
        return loadout.current_ability == AbilityType::Axe;
    }
    
    void interact(EntityID agent_id, EntityManager& entity_manager, 
                 ComponentRegistry& component_registry) override {
        (void)agent_id; 
        (void)component_registry;
        entity_manager.destroy_entity(entity_id_);
    }
    
    std::string get_interaction_prompt() const override {
        return "Press E to chop tree (requires Axe)";
    }
    
    float get_interaction_radius() const override {
        return 48.0f;
    }
};

class DoorObstacle : public IInteractable {
private:
    EntityID entity_id_;
    
public:
    explicit DoorObstacle(EntityID entity_id) : entity_id_(entity_id) {}
    
    bool can_interact(EntityID agent_id, const LoadoutComponent& loadout) const override {
        (void)agent_id; 
        return loadout.current_ability == AbilityType::Keycard;
    }
    
    void interact(EntityID agent_id, EntityManager& entity_manager, 
                 ComponentRegistry& component_registry) override {
        (void)agent_id; 
        (void)entity_manager; 
        auto* door_component = component_registry.get_component<Door>(entity_id_);
        if (door_component && door_component->is_locked) {
            door_component->is_locked = false;
            door_component->is_open = true;
            door_component->animation_progress = 1.0f;
        }
        
        auto* interactable = component_registry.get_component<InteractableComponent>(entity_id_);
        if (interactable) {
            interactable->is_active = false;
            interactable->interaction_text = "Door is open";
        }
    }
    
    std::string get_interaction_prompt() const override {
        return "Press E to unlock door (requires Keycard)";
    }
    
    float get_interaction_radius() const override {
        return 48.0f;
    }
};

class ChasmObstacle : public IInteractable {
private:
    EntityID entity_id_;
    
public:
    explicit ChasmObstacle(EntityID entity_id) : entity_id_(entity_id) {}
    
    bool can_interact(EntityID agent_id, const LoadoutComponent& loadout) const override {
        (void)agent_id; 
        return loadout.current_ability == AbilityType::DoubleJump;
    }
    
    void interact(EntityID agent_id, EntityManager& entity_manager, 
                 ComponentRegistry& component_registry) override {
        (void)entity_manager; 
        auto* physics = component_registry.get_component<PhysicsComponent>(agent_id);
        if (physics) {
            physics->velocity_y = -400.0f; 
            physics->is_grounded = false;
        }
    }
    
    std::string get_interaction_prompt() const override {
        return "Press E to double jump across (requires DoubleJump)";
    }
    
    float get_interaction_radius() const override {
        return 64.0f; 
    }
};