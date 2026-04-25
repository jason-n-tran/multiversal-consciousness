#pragma once

#include "Components.h"
#include <string>

class EntityManager;
class ComponentRegistry;

class IInteractable {
public:
    virtual ~IInteractable() = default;
    
    virtual bool can_interact(EntityID agent_id, const LoadoutComponent& loadout) const = 0;
    
    virtual void interact(EntityID agent_id, EntityManager& entity_manager, 
                         ComponentRegistry& component_registry) = 0;
    
    virtual std::string get_interaction_prompt() const = 0;
    
    virtual float get_interaction_radius() const = 0;
};