#include "QuantumLoadoutSystem.h"
#include "HUDSystem.h"
#include "RealitySystem.h"
#include <algorithm>
#include <cmath>

QuantumLoadoutSystem::QuantumLoadoutSystem() 
    : ability_registry_(std::make_unique<AbilityRegistry>()) {
}

void QuantumLoadoutSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);
    
    reality_loadouts_["A"] = std::unordered_map<EntityID, AbilityType>();
    reality_loadouts_["B"] = std::unordered_map<EntityID, AbilityType>();
    
    ability_registry_->initialize_default_abilities();
    
    initialize_default_abilities();
}

void QuantumLoadoutSystem::update(float delta_time) {
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    update_cooldowns(delta_time);
    
    update_current_abilities();
}

void QuantumLoadoutSystem::shutdown() {
    reality_loadouts_.clear();
    ability_cooldowns_.clear();
    reality_system_ = nullptr;
}

void QuantumLoadoutSystem::set_reality_system(RealitySystem* reality_system) {
    reality_system_ = reality_system;
}

void QuantumLoadoutSystem::set_hud_system(HUDSystem* hud_system) {
    hud_system_ = hud_system;
}

void QuantumLoadoutSystem::switch_reality(Reality new_reality) {
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    auto entities = entity_manager_->get_active_entities();
    
    for (EntityID entity : entities) {
        if (!component_registry_->has_component<LoadoutComponent>(entity)) {
            continue;
        }
        
        auto* loadout = component_registry_->get_component<LoadoutComponent>(entity);
        if (!loadout) {
            continue;
        }
        
        AbilityType new_ability = get_ability_for_reality(entity, new_reality);
        loadout->current_ability = new_ability;
        
        std::string reality_key = get_reality_key(new_reality);
        if (loadout->reality_abilities.find(reality_key) == loadout->reality_abilities.end()) {
            loadout->reality_abilities[reality_key] = AbilityType::None;
        }
        
        if (hud_system_) {
            hud_system_->on_abilities_changed(entity);
        }
    }
}

void QuantumLoadoutSystem::assign_ability(EntityID agent_entity, AbilityType ability, Reality reality) {
    if (!validate_entity_components(agent_entity)) {
        return;
    }
    
    auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    if (!loadout) {
        return;
    }
    
    std::string reality_key = get_reality_key(reality);
    
    loadout->reality_abilities[reality_key] = ability;
    reality_loadouts_[reality_key][agent_entity] = ability;
    
    if (reality_system_ && reality_system_->get_current_reality() == reality) {
        loadout->current_ability = ability;
        
        const auto* ability_def = ability_registry_->get_ability(ability);
        if (ability_def && ability_def->max_uses > 0) {
            loadout->ability_uses = ability_def->max_uses;
        } else {
            loadout->ability_uses = -1; 
        }
        
        loadout->ability_cooldown = 0.0f;
        loadout->ability_ready = true;
        ability_cooldowns_[agent_entity] = 0.0f;
        
        if (hud_system_) {
            hud_system_->on_abilities_changed(agent_entity);
        }
    }
}

AbilityType QuantumLoadoutSystem::get_current_ability(EntityID agent_entity) const {
    if (!validate_entity_components(agent_entity)) {
        return AbilityType::None;
    }
    
    const auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    return loadout ? loadout->current_ability : AbilityType::None;
}

AbilityType QuantumLoadoutSystem::get_ability_for_reality(EntityID agent_entity, Reality reality) const {
    std::string reality_key = get_reality_key(reality);
    
    auto reality_it = reality_loadouts_.find(reality_key);
    if (reality_it != reality_loadouts_.end()) {
        auto entity_it = reality_it->second.find(agent_entity);
        if (entity_it != reality_it->second.end()) {
            return entity_it->second;
        }
    }
    
    if (!validate_entity_components(agent_entity)) {
        return AbilityType::None;
    }
    
    const auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    if (!loadout) {
        return AbilityType::None;
    }
    
    auto it = loadout->reality_abilities.find(reality_key);
    return (it != loadout->reality_abilities.end()) ? it->second : AbilityType::None;
}

bool QuantumLoadoutSystem::can_use_current_ability(EntityID agent_entity) const {
    if (!validate_entity_components(agent_entity)) {
        return false;
    }
    
    const auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    if (!loadout) {
        return false;
    }
    
    const auto* physics = component_registry_->get_component<PhysicsComponent>(agent_entity);
    if (!physics) {
        return false;
    }
    
    return ability_registry_->can_use_ability(loadout->current_ability, *loadout, *physics);
}

bool QuantumLoadoutSystem::use_ability(EntityID agent_entity) {
    if (!can_use_current_ability(agent_entity)) {
        return false;
    }
    
    auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
    if (!loadout) {
        return false;
    }
    
    const auto* ability_def = ability_registry_->get_ability(loadout->current_ability);
    if (!ability_def) {
        return false;
    }
    
    loadout->ability_cooldown = ability_def->cooldown_time;
    ability_cooldowns_[agent_entity] = ability_def->cooldown_time;
    loadout->ability_ready = (ability_def->cooldown_time <= EPSILON);
    
    if (ability_def->max_uses > 0) {
        loadout->ability_uses = std::max(0, loadout->ability_uses - 1);
    }
    
    return true;
}

float QuantumLoadoutSystem::get_ability_cooldown(EntityID agent_entity) const {
    auto it = ability_cooldowns_.find(agent_entity);
    return (it != ability_cooldowns_.end()) ? it->second : 0.0f;
}

void QuantumLoadoutSystem::reset_ability_cooldown(EntityID agent_entity) {
    ability_cooldowns_[agent_entity] = 0.0f;
    
    if (validate_entity_components(agent_entity)) {
        auto* loadout = component_registry_->get_component<LoadoutComponent>(agent_entity);
        if (loadout) {
            loadout->ability_cooldown = 0.0f;
            loadout->ability_ready = true;
        }
    }
}

void QuantumLoadoutSystem::update_cooldowns(float delta_time) {
    for (auto& pair : ability_cooldowns_) {
        if (pair.second > EPSILON) {
            pair.second = std::max(0.0f, pair.second - delta_time);
        }
    }
    
    auto entities = entity_manager_->get_active_entities();
    for (EntityID entity : entities) {
        if (!component_registry_->has_component<LoadoutComponent>(entity)) {
            continue;
        }
        
        auto* loadout = component_registry_->get_component<LoadoutComponent>(entity);
        if (!loadout) {
            continue;
        }
        
        if (loadout->ability_cooldown > EPSILON) {
            loadout->ability_cooldown = std::max(0.0f, loadout->ability_cooldown - delta_time);
            loadout->ability_ready = (loadout->ability_cooldown <= EPSILON);
        } else {
            loadout->ability_ready = true;
        }
    }
}

void QuantumLoadoutSystem::update_current_abilities() {
    if (!reality_system_) {
        return;
    }
    
    Reality current_reality = reality_system_->get_current_reality();
    
    auto entities = entity_manager_->get_active_entities();
    for (EntityID entity : entities) {
        if (!component_registry_->has_component<LoadoutComponent>(entity)) {
            continue;
        }
        
        auto* loadout = component_registry_->get_component<LoadoutComponent>(entity);
        if (!loadout) {
            continue;
        }
        
        AbilityType reality_ability = get_ability_for_reality(entity, current_reality);
        if (loadout->current_ability != reality_ability) {
            loadout->current_ability = reality_ability;
            
            const auto* ability_def = ability_registry_->get_ability(reality_ability);
            if (ability_def && ability_def->max_uses > 0) {
                loadout->ability_uses = ability_def->max_uses;
            } else {
                loadout->ability_uses = -1;
            }
        }
    }
}

bool QuantumLoadoutSystem::validate_entity_components(EntityID entity) const {
    return entity_manager_ && component_registry_ && 
           entity_manager_->is_valid(entity) &&
           component_registry_->has_component<LoadoutComponent>(entity);
}

std::string QuantumLoadoutSystem::get_reality_key(Reality reality) const {
    return (reality == Reality::A) ? "A" : "B";
}

void QuantumLoadoutSystem::initialize_default_abilities() {
}