#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "RealityManager.h"
#include "Components.h"
#include "AbilitySystem.h"
#include <unordered_map>
#include <memory>

class RealitySystem;
class HUDSystem;

class QuantumLoadoutSystem : public ISystem {
private:
    std::unique_ptr<AbilityRegistry> ability_registry_;
    RealitySystem* reality_system_{nullptr};  
    HUDSystem* hud_system_{nullptr};       
    
    std::unordered_map<std::string, std::unordered_map<EntityID, AbilityType>> reality_loadouts_;
    
    std::unordered_map<EntityID, float> ability_cooldowns_;
    
    static constexpr float EPSILON = 1e-6f;
    
public:
    QuantumLoadoutSystem();
    
    ~QuantumLoadoutSystem() override = default;
    
    QuantumLoadoutSystem(const QuantumLoadoutSystem&) = delete;
    QuantumLoadoutSystem& operator=(const QuantumLoadoutSystem&) = delete;
    
    QuantumLoadoutSystem(QuantumLoadoutSystem&&) noexcept = default;
    QuantumLoadoutSystem& operator=(QuantumLoadoutSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    void set_reality_system(RealitySystem* reality_system);
    
    void set_hud_system(HUDSystem* hud_system);
    
    void switch_reality(Reality new_reality);
    
    void assign_ability(EntityID agent_entity, AbilityType ability, Reality reality);
    
    AbilityType get_current_ability(EntityID agent_entity) const;
    
    AbilityType get_ability_for_reality(EntityID agent_entity, Reality reality) const;
    
    bool can_use_current_ability(EntityID agent_entity) const;
    
    bool use_ability(EntityID agent_entity);
    
    const AbilityRegistry& get_ability_registry() const {
        return *ability_registry_;
    }
    
    float get_ability_cooldown(EntityID agent_entity) const;
    
    void reset_ability_cooldown(EntityID agent_entity);
    
private:
    void update_cooldowns(float delta_time);
    
    void update_current_abilities();
    
    bool validate_entity_components(EntityID entity) const;
    
    std::string get_reality_key(Reality reality) const;
    
    void initialize_default_abilities();
};