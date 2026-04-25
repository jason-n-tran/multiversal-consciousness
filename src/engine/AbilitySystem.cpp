#include "AbilitySystem.h"

void AbilityRegistry::register_ability(const AbilityDefinition& ability) {
    abilities_[ability.type] = ability;
}

const AbilityDefinition* AbilityRegistry::get_ability(AbilityType type) const {
    auto it = abilities_.find(type);
    return (it != abilities_.end()) ? &it->second : nullptr;
}

bool AbilityRegistry::can_use_ability(AbilityType type, const LoadoutComponent& loadout, 
                                     const PhysicsComponent& physics) const {
    if (!loadout.ability_ready || loadout.ability_cooldown > 0.0f) {
        return false;
    }
    
    if (loadout.current_ability != type) {
        return false;
    }
    
    const AbilityDefinition* ability = get_ability(type);
    if (!ability) {
        return false;
    }
    
    if (ability->requires_ground && !physics.is_grounded) {
        return false;
    }
    
    if (ability->max_uses > 0 && loadout.ability_uses >= ability->max_uses) {
        return false;
    }
    
    return true;
}

void AbilityRegistry::initialize_default_abilities() {
    register_ability({AbilityType::None, "None", "No special ability", 0.0f, -1, false, 0.0f});
    register_ability({AbilityType::Axe, "Axe", "Chop down trees", 0.5f, -1, false, 0.0f});
    register_ability({AbilityType::Keycard, "Keycard", "Open locked doors", 0.0f, -1, false, 0.0f});
    register_ability({AbilityType::DoubleJump, "Double Jump", "Jump twice in mid-air", 0.2f, -1, false, 0.0f});
    register_ability({AbilityType::Dash, "Dash", "Quick horizontal movement", 1.0f, -1, false, 0.0f});
    register_ability({AbilityType::WaterWalk, "Water Walk", "Walk on water surfaces", 0.0f, -1, false, 0.0f});
    register_ability({AbilityType::PhaseShift, "Phase Shift", "Pass through certain obstacles", 2.0f, 3, false, 10.0f});
}