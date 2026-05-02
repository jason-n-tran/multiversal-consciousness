#pragma once

#include "IInteractable.h"
#include "Components.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include <string>
#include <iostream>

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
    mutable std::string feedback_message_;
    mutable bool has_feedback_{false};
    
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
            auto* bbox = component_registry.get_component<BoundingBoxComponent>(entity_id_);
            if (bbox) {
                bbox->is_solid = false; 
            }
            
            feedback_message_ = "Door Unlocked!";
            has_feedback_ = true;
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
    
    std::string get_feedback_message() const override {
        if (has_feedback_) {
            has_feedback_ = false;
            return feedback_message_;
        }
        return "";
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

class SwitchObstacle : public IInteractable {
private:
    EntityID entity_id_;
    mutable std::string feedback_message_;
    mutable bool has_feedback_{false};
    
    
    void apply_switch_effects(const EnvironmentalSwitch& env_switch, ComponentRegistry& component_registry) {
        feedback_message_ = env_switch.is_activated ? "Switch Activated" : "Switch Deactivated";

        
        if (env_switch.target_entity_type == "door") {
            const auto* door_container = component_registry.get_all_components<Door>();
            if (door_container) {
                const auto& entities = door_container->get_entities();
                for (EntityID entity : entities) {
                    auto* door = component_registry.get_component<Door>(entity);
                    if (door) {
                        if (env_switch.is_activated) {
                            if (door->is_locked) {
                                door->is_locked = false;
                                door->is_open = true;
                                door->animation_progress = 1.0f;
                                
                                auto* bbox = component_registry.get_component<BoundingBoxComponent>(entity);
                                if (bbox) {
                                    bbox->is_solid = false;
                                }
                                
                                std::cout << "Switch unlocked door " << entity << std::endl;
                            }
                        } else {
                            auto* interactable = component_registry.get_component<InteractableComponent>(entity);
                            bool unlocked_by_keycard = (interactable && !interactable->is_active);
                            
                            if (!unlocked_by_keycard) {
                                door->is_locked = true;
                                door->is_open = false;
                                door->animation_progress = 0.0f;
                                
                                auto* bbox = component_registry.get_component<BoundingBoxComponent>(entity);
                                if (bbox) {
                                    bbox->is_solid = true;
                                }
                                
                                std::cout << "Switch locked door " << entity << std::endl;
                            } else {
                                std::cout << "Switch cannot lock door " << entity << " (unlocked by keycard)" << std::endl;
                            }
                        }
                    }
                }
                if (env_switch.is_activated) {
                    feedback_message_ = "Switch Activated: Doors Unlocked";
                } else {
                    feedback_message_ = "Switch Deactivated: Doors Locked";
                }
            }
        } else if (env_switch.target_entity_type == "water") {
            const auto* water_container = component_registry.get_all_components<WaterLevel>();
            if (water_container) {
                const auto& entities = water_container->get_entities();
                for (EntityID entity : entities) {
                    auto* water = component_registry.get_component<WaterLevel>(entity);
                    auto* bbox = component_registry.get_component<BoundingBoxComponent>(entity);
                    
                    if (water && bbox) {
                        if (env_switch.is_activated) {
                            bbox->is_solid = true;
                            bbox->is_trigger = false;
                            water->target_level = 0.0f;
                            water->is_draining = true;
                            water->is_filling = false;
                            std::cout << "Switch solidified water " << entity << " (now behaves like wall)" << std::endl;
                        } else {
                            bbox->is_solid = false;
                            bbox->is_trigger = true;
                            water->target_level = 32.0f;
                            water->is_draining = false;
                            water->is_filling = true;
                            std::cout << "Switch liquified water " << entity << " (now normal water)" << std::endl;
                        }
                    }
                }
                if (env_switch.is_activated) {
                    feedback_message_ = "Switch Activated: Water Solidified";
                } else {
                    feedback_message_ = "Switch Deactivated: Water Liquified";
                }
            }
        }
    }
    
public:
    explicit SwitchObstacle(EntityID entity_id) : entity_id_(entity_id) {}
    
    bool can_interact(EntityID agent_id, const LoadoutComponent& loadout) const override {
        (void)agent_id; 
        (void)loadout; 
        return true; 
    }
    
    void interact(EntityID agent_id, EntityManager& entity_manager, 
                 ComponentRegistry& component_registry) override {
        (void)agent_id; 
        (void)entity_manager; 
        
        auto* env_switch = component_registry.get_component<EnvironmentalSwitch>(entity_id_);
        if (env_switch) {
            env_switch->is_activated = !env_switch->is_activated;
            
            apply_switch_effects(*env_switch, component_registry);
            
            std::cout << "Switch " << entity_id_ << " " 
                      << (env_switch->is_activated ? "activated" : "deactivated") << std::endl;
            
            has_feedback_ = true;
        }
    }
    
    std::string get_interaction_prompt() const override {
        return "Press E to activate switch";
    }
    
    float get_interaction_radius() const override {
        return 48.0f;
    }
    
    std::string get_feedback_message() const override {
        if (has_feedback_) {
            has_feedback_ = false;
            return feedback_message_;
        }
        return "";
    }
};