#include "PuzzleValidator.h"
#include "LevelLoader.h"
#include "PuzzleSystem.h"
#include <iostream>
#include <sstream>

PuzzleValidator::PuzzleValidator(EntityManager* entity_manager, ComponentRegistry* component_registry)
    : entity_manager_(entity_manager), component_registry_(component_registry) {
    initialize_validators();
}

void PuzzleValidator::initialize_validators() {
    level_validators_["agent_has_item"] = [this](const LevelCondition& condition) {
        return validate_agent_has_item_level(condition);
    };
    
    level_validators_["agent_has_ability"] = [this](const LevelCondition& condition) {
        return validate_agent_has_ability_level(condition);
    };
    
    level_validators_["door_state"] = [this](const LevelCondition& condition) {
        return validate_door_state_level(condition);
    };
    
    level_validators_["water_level"] = [this](const LevelCondition& condition) {
        return validate_water_level_level(condition);
    };
    
    level_validators_["switch_state"] = [this](const LevelCondition& condition) {
        return validate_switch_state_level(condition);
    };
    
    level_validators_["quantum_node_activated"] = [this](const LevelCondition& condition) {
        return validate_quantum_node_activated_level(condition);
    };
    
    level_validators_["multi_agent_coordination"] = [this](const LevelCondition& condition) {
        return validate_multi_agent_coordination_level(condition);
    };

        level_validators_["agent_position"] = [this](const LevelCondition& condition) {
        return validate_agent_position_level(condition);
    };
    
    level_validators_["trigger_activated"] = [this](const LevelCondition& condition) {
        return validate_trigger_activated_level(condition);
    };
    
    puzzle_validators_["agent_has_item"] = [this](const PuzzleCondition& condition) {
        return validate_agent_has_item(condition);
    };
    
    puzzle_validators_["agent_has_ability"] = [this](const PuzzleCondition& condition) {
        return validate_agent_has_ability(condition);
    };
    
    puzzle_validators_["door_state"] = [this](const PuzzleCondition& condition) {
        return validate_door_state(condition);
    };
    
    puzzle_validators_["water_level"] = [this](const PuzzleCondition& condition) {
        return validate_water_level(condition);
    };
    
    puzzle_validators_["switch_state"] = [this](const PuzzleCondition& condition) {
        return validate_switch_state(condition);
    };
    
    puzzle_validators_["quantum_node_activated"] = [this](const PuzzleCondition& condition) {
        return validate_quantum_node_activated(condition);
    };
    
    puzzle_validators_["multi_agent_coordination"] = [this](const PuzzleCondition& condition) {
        return validate_multi_agent_coordination(condition);
    };
}

ValidationResult PuzzleValidator::validate_condition(const PuzzleCondition& condition) {
    auto it = puzzle_validators_.find(condition.condition_type);
    if (it != puzzle_validators_.end()) {
        return it->second(condition);
    }
    
    ValidationResult result;
    result.is_valid = false;
    result.condition_type = condition.condition_type;
    result.target = ""; 
    result.expected_value = "";
    result.actual_value = "unknown";
    result.description = "Unknown condition type: " + condition.condition_type;
    return result;
}

ValidationResult PuzzleValidator::validate_condition(const LevelCondition& condition) {
    auto it = level_validators_.find(condition.type);
    if (it != level_validators_.end()) {
        return it->second(condition);
    }
    
    ValidationResult result;
    result.is_valid = false;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    result.actual_value = "unknown";
    result.description = "Unknown condition type: " + condition.type;
    return result;
}

PuzzleCondition PuzzleValidator::convert_level_condition(const LevelCondition& level_condition) {
    PuzzleCondition puzzle_condition(
        level_condition.type + "_" + level_condition.target,
        level_condition.type,         
        {},                                      
        nullptr                    
    );
    
    if (!level_condition.target.empty()) {
        puzzle_condition.parameters["target"] = std::stof(level_condition.target);
    }
    if (!level_condition.value.empty()) {
        puzzle_condition.parameters["value"] = std::stof(level_condition.value);
    }
    
    return puzzle_condition;
}

std::vector<ValidationResult> PuzzleValidator::validate_all_conditions(const std::vector<PuzzleCondition>& conditions) {
    std::vector<ValidationResult> results;
    results.reserve(conditions.size());
    
    for (const auto& condition : conditions) {
        results.push_back(validate_condition(condition));
    }
    
    return results;
}

std::vector<ValidationResult> PuzzleValidator::validate_all_conditions(const std::vector<LevelCondition>& conditions) {
    std::vector<ValidationResult> results;
    results.reserve(conditions.size());
    
    for (const auto& condition : conditions) {
        results.push_back(validate_condition(condition));
    }
    
    return results;
}

bool PuzzleValidator::are_all_conditions_met(const std::vector<PuzzleCondition>& conditions) {
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        if (!result.is_valid) {
            return false;
        }
    }
    return true;
}

bool PuzzleValidator::are_all_conditions_met(const std::vector<LevelCondition>& conditions) {
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        if (!result.is_valid) {
            return false;
        }
    }
    return true;
}

float PuzzleValidator::get_completion_percentage(const std::vector<PuzzleCondition>& conditions) {
    if (conditions.empty()) {
        return 1.0f;
    }
    
    int met_conditions = 0;
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        if (result.is_valid) {
            met_conditions++;
        }
    }
    
    return static_cast<float>(met_conditions) / static_cast<float>(conditions.size());
}

float PuzzleValidator::get_completion_percentage(const std::vector<LevelCondition>& conditions) {
    if (conditions.empty()) {
        return 1.0f;
    }
    
    int met_conditions = 0;
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        if (result.is_valid) {
            met_conditions++;
        }
    }
    
    return static_cast<float>(met_conditions) / static_cast<float>(conditions.size());
}

std::vector<std::string> PuzzleValidator::get_status_report(const std::vector<PuzzleCondition>& conditions) {
    std::vector<std::string> report;
    
    report.push_back("=== Puzzle Validation Report ===");
    
    if (conditions.empty()) {
        report.push_back("No conditions to validate");
        return report;
    }
    
    int met_conditions = 0;
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        
        std::ostringstream line;
        line << (result.is_valid ? "[✓] " : "[✗] ");
        line << result.condition_type << " " << condition.condition_id;
        
        if (!result.is_valid && !result.actual_value.empty()) {
            line << " (actual: " << result.actual_value << ")";
        }
        
        if (!result.description.empty()) {
            line << " - " << result.description;
        }
        
        report.push_back(line.str());
        
        if (result.is_valid) {
            met_conditions++;
        }
    }
    
    float percentage = static_cast<float>(met_conditions) / static_cast<float>(conditions.size()) * 100.0f;
    report.push_back("");
    report.push_back("Progress: " + std::to_string(met_conditions) + "/" + std::to_string(conditions.size()) + 
                     " (" + std::to_string(static_cast<int>(percentage)) + "%)");
    
    return report;
}

std::vector<std::string> PuzzleValidator::get_status_report(const std::vector<LevelCondition>& conditions) {
    std::vector<std::string> report;
    
    report.push_back("=== Puzzle Validation Report ===");
    
    if (conditions.empty()) {
        report.push_back("No conditions to validate");
        return report;
    }
    
    int met_conditions = 0;
    for (const auto& condition : conditions) {
        ValidationResult result = validate_condition(condition);
        
        std::ostringstream line;
        line << (result.is_valid ? "[✓] " : "[✗] ");
        line << result.condition_type << " " << result.target << " " << result.expected_value;
        
        if (!result.is_valid && !result.actual_value.empty()) {
            line << " (actual: " << result.actual_value << ")";
        }
        
        if (!result.description.empty()) {
            line << " - " << result.description;
        }
        
        report.push_back(line.str());
        
        if (result.is_valid) {
            met_conditions++;
        }
    }
    
    float percentage = static_cast<float>(met_conditions) / static_cast<float>(conditions.size()) * 100.0f;
    report.push_back("");
    report.push_back("Progress: " + std::to_string(met_conditions) + "/" + std::to_string(conditions.size()) + 
                     " (" + std::to_string(static_cast<int>(percentage)) + "%)");
    
    return report;
}

ValidationResult PuzzleValidator::validate_agent_has_item(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id; 
    result.expected_value = ""; 
    
    uint8_t agent_number = 1; 
    auto target_it = condition.parameters.find("target");
    if (target_it != condition.parameters.end()) {
        agent_number = static_cast<uint8_t>(target_it->second);
    }
    
    EntityID agent_entity = find_agent_by_number(agent_number);
    
    if (agent_entity == 0) {
        result.is_valid = false;
        result.actual_value = "agent_not_found";
        result.description = "Agent " + std::to_string(agent_number) + " not found";
        return result;
    }
    
    const Inventory* inventory = component_registry_->get_component<Inventory>(agent_entity);
    if (!inventory) {
        result.is_valid = false;
        result.actual_value = "no_inventory";
        result.description = "Agent " + std::to_string(agent_number) + " has no inventory";
        return result;
    }
    
    bool has_items = !inventory->items.empty();
    result.is_valid = has_items;
    result.actual_value = has_items ? "has_items" : "no_items";
    result.description = has_items ? "Agent has items" : "Agent has no items";
    
    return result;
}

ValidationResult PuzzleValidator::validate_agent_has_ability(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    uint8_t agent_number = 1;
    auto target_it = condition.parameters.find("target");
    if (target_it != condition.parameters.end()) {
        agent_number = static_cast<uint8_t>(target_it->second);
    }
    
    EntityID agent_entity = find_agent_by_number(agent_number);
    
    if (agent_entity == 0) {
        result.is_valid = false;
        result.actual_value = "agent_not_found";
        result.description = "Agent " + std::to_string(agent_number) + " not found";
        return result;
    }
    
    // Check if agent has inventory with abilities
    const Inventory* inventory = component_registry_->get_component<Inventory>(agent_entity);
    if (!inventory) {
        result.is_valid = false;
        result.actual_value = "no_inventory";
        result.description = "Agent " + std::to_string(agent_number) + " has no inventory";
        return result;
    }
    
    bool has_abilities = !inventory->abilities.empty();
    result.is_valid = has_abilities;
    result.actual_value = has_abilities ? "has_abilities" : "no_abilities";
    result.description = has_abilities ? "Agent has abilities" : "Agent has no abilities";
    
    return result;
}

ValidationResult PuzzleValidator::validate_door_state(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    EntityID door_entity = find_entity_by_type("door");
    
    if (door_entity == 0) {
        result.is_valid = false;
        result.actual_value = "door_not_found";
        result.description = "Door not found";
        return result;
    }
    
    const Door* door = component_registry_->get_component<Door>(door_entity);
    if (!door) {
        result.is_valid = false;
        result.actual_value = "no_door_component";
        result.description = "Entity has no door component";
        return result;
    }
    
    bool condition_met = door->is_open;
    result.is_valid = condition_met;
    result.actual_value = door->is_open ? "open" : "closed";
    result.description = condition_met ? "Door is open" : "Door is closed";
    
    return result;
}

ValidationResult PuzzleValidator::validate_water_level(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    EntityID water_entity = find_entity_by_type("water");
    
    if (water_entity == 0) {
        result.is_valid = false;
        result.actual_value = "water_not_found";
        result.description = "Water entity not found";
        return result;
    }
    
    const WaterLevel* water = component_registry_->get_component<WaterLevel>(water_entity);
    if (!water) {
        result.is_valid = false;
        result.actual_value = "no_water_component";
        result.description = "Entity has no water level component";
        return result;
    }
    
    float min_level = 50.0f;
    auto value_it = condition.parameters.find("value");
    if (value_it != condition.parameters.end()) {
        min_level = value_it->second;
    }
    
    bool condition_met = water->current_level >= min_level;
    result.is_valid = condition_met;
    result.actual_value = std::to_string(water->current_level);
    result.description = condition_met ? "Water level sufficient" : "Water level too low";
    
    return result;
}

ValidationResult PuzzleValidator::validate_switch_state(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    EntityID switch_entity = find_entity_by_type("switch");
    
    if (switch_entity == 0) {
        result.is_valid = false;
        result.actual_value = "switch_not_found";
        result.description = "Switch entity not found";
        return result;
    }
    
    const EnvironmentalSwitch* env_switch = component_registry_->get_component<EnvironmentalSwitch>(switch_entity);
    if (!env_switch) {
        result.is_valid = false;
        result.actual_value = "no_switch_component";
        result.description = "Entity has no switch component";
        return result;
    }
    
    bool condition_met = env_switch->is_activated;
    
    result.is_valid = condition_met;
    result.actual_value = env_switch->is_activated ? "activated" : "deactivated";
    result.description = condition_met ? "Switch is activated" : "Switch is not activated";
    
    return result;
}

ValidationResult PuzzleValidator::validate_quantum_node_activated(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    auto entities = entity_manager_->get_active_entities();
    EntityID quantum_entity = 0;
    
    for (EntityID entity : entities) {
        if (component_registry_->has_component<QuantumNode>(entity)) {
            quantum_entity = entity;
            break; 
        }
    }
    
    if (quantum_entity == 0) {
        result.is_valid = false;
        result.actual_value = "quantum_node_not_found";
        result.description = "Quantum node not found";
        return result;
    }
    
    const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(quantum_entity);
    if (!quantum_node) {
        result.is_valid = false;
        result.actual_value = "no_quantum_component";
        result.description = "Entity has no quantum node component";
        return result;
    }
    
    bool condition_met = quantum_node->is_activated;
    
    result.is_valid = condition_met;
    result.actual_value = quantum_node->is_activated ? "activated" : "not_activated";
    result.description = condition_met ? "Quantum node is activated" : "Quantum node is not activated";
    
    return result;
}

ValidationResult PuzzleValidator::validate_multi_agent_coordination(const PuzzleCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.condition_type;
    result.target = condition.condition_id;
    result.expected_value = "";
    
    auto entities = entity_manager_->get_active_entities();
    int agents_with_items = 0;
    int total_agents = 0;
    
    for (EntityID entity : entities) {
        if (component_registry_->has_component<Agent>(entity)) {
            total_agents++;
            const Inventory* inventory = component_registry_->get_component<Inventory>(entity);
            if (inventory && (!inventory->items.empty() || !inventory->abilities.empty())) {
                agents_with_items++;
            }
        }
    }
    
    bool condition_met = (agents_with_items >= 2 && total_agents >= 2);
    
    result.is_valid = condition_met;
    result.actual_value = std::to_string(agents_with_items) + "/" + std::to_string(total_agents);
    result.description = condition_met ? "Multi-agent coordination detected" : "Insufficient agent coordination";
    
    return result;
}

EntityID PuzzleValidator::find_agent_by_number(uint8_t agent_number) {
    if (!entity_manager_ || !component_registry_) {
        return 0;
    }
    
    auto entities = entity_manager_->get_active_entities();
    for (EntityID entity : entities) {
        if (component_registry_->has_component<Agent>(entity)) {
            const Agent* agent = component_registry_->get_component<Agent>(entity);
            if (agent && agent->agent_number == agent_number) {
                return entity;
            }
        }
    }
    
    return 0;
}

EntityID PuzzleValidator::find_entity_by_type(const std::string& entity_type, const std::unordered_map<std::string, std::string>& /* properties */) {
    if (!entity_manager_ || !component_registry_) {
        return 0;
    }
    
    auto entities = entity_manager_->get_active_entities();
    for (EntityID entity : entities) {
        if (entity_type == "door" && component_registry_->has_component<Door>(entity)) {
            return entity;
        } else if (entity_type == "water" && component_registry_->has_component<WaterLevel>(entity)) {
            return entity;
        } else if (entity_type == "switch" && component_registry_->has_component<EnvironmentalSwitch>(entity)) {
            return entity;
        }
    }
    
    return 0;
}

void PuzzleValidator::register_validator(const std::string& condition_type, std::function<ValidationResult(const PuzzleCondition&)> validator) {
    puzzle_validators_[condition_type] = validator;
}

ValidationResult PuzzleValidator::validate_agent_has_item_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    uint8_t agent_number = static_cast<uint8_t>(std::stoi(condition.target));
    EntityID agent_entity = find_agent_by_number(agent_number);
    
    if (agent_entity == 0) {
        result.is_valid = false;
        result.actual_value = "agent_not_found";
        result.description = "Agent " + condition.target + " not found";
        return result;
    }
    
    const Inventory* inventory = component_registry_->get_component<Inventory>(agent_entity);
    if (!inventory) {
        result.is_valid = false;
        result.actual_value = "no_inventory";
        result.description = "Agent " + condition.target + " has no inventory";
        return result;
    }
    
    bool has_item = inventory->items.count(condition.value) > 0;
    result.is_valid = has_item;
    result.actual_value = has_item ? condition.value : "missing";
    result.description = has_item ? "Agent has required item" : "Agent missing required item";
    
    return result;
}

ValidationResult PuzzleValidator::validate_agent_has_ability_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    uint8_t agent_number = static_cast<uint8_t>(std::stoi(condition.target));
    EntityID agent_entity = find_agent_by_number(agent_number);
    
    if (agent_entity == 0) {
        result.is_valid = false;
        result.actual_value = "agent_not_found";
        result.description = "Agent " + condition.target + " not found";
        return result;
    }
    
    const Inventory* inventory = component_registry_->get_component<Inventory>(agent_entity);
    if (!inventory) {
        result.is_valid = false;
        result.actual_value = "no_inventory";
        result.description = "Agent " + condition.target + " has no inventory";
        return result;
    }
    
    bool has_ability = inventory->abilities.count(condition.value) > 0;
    result.is_valid = has_ability;
    result.actual_value = has_ability ? condition.value : "missing";
    result.description = has_ability ? "Agent has required ability" : "Agent missing required ability";
    
    return result;
}

ValidationResult PuzzleValidator::validate_door_state_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    EntityID door_entity = find_entity_by_type("door");
    
    if (door_entity == 0) {
        result.is_valid = false;
        result.actual_value = "door_not_found";
        result.description = "Door not found";
        return result;
    }
    
    const Door* door = component_registry_->get_component<Door>(door_entity);
    if (!door) {
        result.is_valid = false;
        result.actual_value = "no_door_component";
        result.description = "Entity has no door component";
        return result;
    }
    
    bool condition_met = false;
    if (condition.value == "open") {
        condition_met = door->is_open;
        result.actual_value = door->is_open ? "open" : "closed";
    } else if (condition.value == "closed") {
        condition_met = !door->is_open;
        result.actual_value = door->is_open ? "open" : "closed";
    } else if (condition.value == "unlocked") {
        condition_met = !door->is_locked;
        result.actual_value = door->is_locked ? "locked" : "unlocked";
    }
    
    result.is_valid = condition_met;
    result.description = condition_met ? "Door state matches requirement" : "Door state does not match requirement";
    
    return result;
}

ValidationResult PuzzleValidator::validate_water_level_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    EntityID water_entity = find_entity_by_type("water");
    
    if (water_entity == 0) {
        result.is_valid = false;
        result.actual_value = "water_not_found";
        result.description = "Water entity not found";
        return result;
    }
    
    const WaterLevel* water = component_registry_->get_component<WaterLevel>(water_entity);
    if (!water) {
        result.is_valid = false;
        result.actual_value = "no_water_component";
        result.description = "Entity has no water level component";
        return result;
    }
    
    float expected_level = std::stof(condition.value);
    float tolerance = 5.0f; 
    
    bool condition_met = std::abs(water->current_level - expected_level) <= tolerance;
    result.is_valid = condition_met;
    result.actual_value = std::to_string(water->current_level);
    result.description = condition_met ? "Water level within acceptable range" : "Water level outside acceptable range";
    
    return result;
}

ValidationResult PuzzleValidator::validate_switch_state_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    EntityID switch_entity = find_entity_by_type("switch");
    
    if (switch_entity == 0) {
        result.is_valid = false;
        result.actual_value = "switch_not_found";
        result.description = "Switch entity not found";
        return result;
    }
    
    const EnvironmentalSwitch* env_switch = component_registry_->get_component<EnvironmentalSwitch>(switch_entity);
    if (!env_switch) {
        result.is_valid = false;
        result.actual_value = "no_switch_component";
        result.description = "Entity has no switch component";
        return result;
    }
    
    bool expected_state = (condition.value == "activated" || condition.value == "true");
    bool condition_met = (env_switch->is_activated == expected_state);
    
    result.is_valid = condition_met;
    result.actual_value = env_switch->is_activated ? "activated" : "deactivated";
    result.description = condition_met ? "Switch state matches requirement" : "Switch state does not match requirement";
    
    return result;
}

ValidationResult PuzzleValidator::validate_quantum_node_activated_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    auto entities = entity_manager_->get_active_entities();
    EntityID quantum_entity = 0;
    
    for (EntityID entity : entities) {
        if (component_registry_->has_component<QuantumNode>(entity)) {
            quantum_entity = entity;
            break; 
        }
    }
    
    if (quantum_entity == 0) {
        result.is_valid = false;
        result.actual_value = "quantum_node_not_found";
        result.description = "Quantum node not found";
        return result;
    }
    
    const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(quantum_entity);
    if (!quantum_node) {
        result.is_valid = false;
        result.actual_value = "no_quantum_component";
        result.description = "Entity has no quantum node component";
        return result;
    }
    
    bool expected_state = (condition.value == "activated" || condition.value == "true");
    bool condition_met = (quantum_node->is_activated == expected_state);
    
    result.is_valid = condition_met;
    result.actual_value = quantum_node->is_activated ? "activated" : "not_activated";
    result.description = condition_met ? "Quantum node state matches requirement" : "Quantum node state does not match requirement";
    
    return result;
}
ValidationResult PuzzleValidator::validate_multi_agent_coordination_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    auto entities = entity_manager_->get_active_entities();
    int agents_with_items = 0;
    int total_agents = 0;
    
    for (EntityID entity : entities) {
        if (component_registry_->has_component<Agent>(entity)) {
            total_agents++;
            const Inventory* inventory = component_registry_->get_component<Inventory>(entity);
            if (inventory && (!inventory->items.empty() || !inventory->abilities.empty())) {
                agents_with_items++;
            }
        }
    }
    
    bool condition_met = (agents_with_items >= 2 && total_agents >= 2);
    
    result.is_valid = condition_met;
    result.actual_value = std::to_string(agents_with_items) + "/" + std::to_string(total_agents);
    result.description = condition_met ? "Multi-agent coordination detected" : "Insufficient agent coordination";
    
    return result;
}

ValidationResult PuzzleValidator::validate_agent_position_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    uint8_t agent_number = static_cast<uint8_t>(std::stoi(condition.target));
    EntityID agent_entity = find_agent_by_number(agent_number);
    
    if (agent_entity == 0) {
        result.is_valid = false;
        result.actual_value = "agent_not_found";
        result.description = "Agent " + condition.target + " not found";
        return result;
    }
    
    const Transform* transform = component_registry_->get_component<Transform>(agent_entity);
    if (!transform) {
        result.is_valid = false;
        result.actual_value = "no_transform";
        result.description = "Agent " + condition.target + " has no transform component";
        return result;
    }
    
    std::string condition_str = condition.value;
    bool condition_met = false;
    
    if (condition_str.find("x>") == 0) {
        float threshold = std::stof(condition_str.substr(2));
        condition_met = transform->x > threshold;
        result.actual_value = "x=" + std::to_string(transform->x);
    } else if (condition_str.find("x<") == 0) {
        float threshold = std::stof(condition_str.substr(2));
        condition_met = transform->x < threshold;
        result.actual_value = "x=" + std::to_string(transform->x);
    } else if (condition_str.find("x>=") == 0) {
        float threshold = std::stof(condition_str.substr(3));
        condition_met = transform->x >= threshold;
        result.actual_value = "x=" + std::to_string(transform->x);
    } else if (condition_str.find("x<=") == 0) {
        float threshold = std::stof(condition_str.substr(3));
        condition_met = transform->x <= threshold;
        result.actual_value = "x=" + std::to_string(transform->x);
    } else if (condition_str.find("y>") == 0) {
        float threshold = std::stof(condition_str.substr(2));
        condition_met = transform->y > threshold;
        result.actual_value = "y=" + std::to_string(transform->y);
    } else if (condition_str.find("y<") == 0) {
        float threshold = std::stof(condition_str.substr(2));
        condition_met = transform->y < threshold;
        result.actual_value = "y=" + std::to_string(transform->y);
    } else if (condition_str.find("y>=") == 0) {
        float threshold = std::stof(condition_str.substr(3));
        condition_met = transform->y >= threshold;
        result.actual_value = "y=" + std::to_string(transform->y);
    } else if (condition_str.find("y<=") == 0) {
        float threshold = std::stof(condition_str.substr(3));
        condition_met = transform->y <= threshold;
        result.actual_value = "y=" + std::to_string(transform->y);
    } else {
        result.is_valid = false;
        result.actual_value = "invalid_condition";
        result.description = "Invalid position condition format: " + condition_str;
        return result;
    }
    
    result.is_valid = condition_met;
    result.description = condition_met ? "Agent position meets requirement" : "Agent position does not meet requirement";
    
    return result;
}

ValidationResult PuzzleValidator::validate_trigger_activated_level(const LevelCondition& condition) {
    ValidationResult result;
    result.condition_type = condition.type;
    result.target = condition.target;
    result.expected_value = condition.value;
    
    auto entities = entity_manager_->get_active_entities();
    EntityID trigger_entity = 0;
    
    for (EntityID entity : entities) {
        if (component_registry_->has_component<Trigger>(entity)) {
            const Trigger* trigger = component_registry_->get_component<Trigger>(entity);
            if (trigger && trigger->trigger_type == condition.target) {
                trigger_entity = entity;
                break;
            }
        }
    }
    
    if (trigger_entity == 0) {
        result.is_valid = false;
        result.actual_value = "trigger_not_found";
        result.description = "Trigger '" + condition.target + "' not found";
        return result;
    }
    
    const Trigger* trigger = component_registry_->get_component<Trigger>(trigger_entity);
    if (!trigger) {
        result.is_valid = false;
        result.actual_value = "no_trigger_component";
        result.description = "Entity has no trigger component";
        return result;
    }
    
    bool expected_state = (condition.value == "true" || condition.value == "activated");
    bool condition_met = (trigger->has_been_triggered == expected_state);
    
    result.is_valid = condition_met;
    result.actual_value = trigger->has_been_triggered ? "activated" : "not_activated";
    result.description = condition_met ? "Trigger state matches requirement" : "Trigger state does not match requirement";
    
    return result;
}