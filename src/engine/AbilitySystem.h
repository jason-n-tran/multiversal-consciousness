#pragma once

#include "Components.h"
#include <unordered_map>
#include <string>

struct AbilityDefinition {
    AbilityType type;
    std::string name;
    std::string description;
    float cooldown_time{0.0f};
    int max_uses{-1};  // -1 for unlimited
    bool requires_ground{false};
    float energy_cost{0.0f};
};

class AbilityRegistry {
private:
    std::unordered_map<AbilityType, AbilityDefinition> abilities_;
    
public:
    void register_ability(const AbilityDefinition& ability);
    
    const AbilityDefinition* get_ability(AbilityType type) const;
    
    bool can_use_ability(AbilityType type, const LoadoutComponent& loadout, 
                        const PhysicsComponent& physics) const;
    
    void initialize_default_abilities();
};