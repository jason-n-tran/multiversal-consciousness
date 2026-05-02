#pragma once

#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

struct LevelCondition;
struct PuzzleCondition;

struct ValidationResult {
    bool is_valid{false};
    std::string condition_type;
    std::string target;
    std::string expected_value;
    std::string actual_value;
    std::string description;
};

class PuzzleValidator {
private:
    EntityManager* entity_manager_{nullptr};
    ComponentRegistry* component_registry_{nullptr};
    std::unordered_map<std::string, std::function<ValidationResult(const LevelCondition&)>> level_validators_;
    std::unordered_map<std::string, std::function<ValidationResult(const PuzzleCondition&)>> puzzle_validators_;
    
    void initialize_validators();

    PuzzleCondition convert_level_condition(const LevelCondition& level_condition);
    
    ValidationResult validate_agent_has_item_level(const LevelCondition& condition);

    ValidationResult validate_agent_has_ability_level(const LevelCondition& condition);

    ValidationResult validate_door_state_level(const LevelCondition& condition);

    ValidationResult validate_water_level_level(const LevelCondition& condition);

    ValidationResult validate_switch_state_level(const LevelCondition& condition);

    ValidationResult validate_quantum_node_activated_level(const LevelCondition& condition);

    ValidationResult validate_multi_agent_coordination_level(const LevelCondition& condition);

    ValidationResult validate_agent_position_level(const LevelCondition& condition);
    
    ValidationResult validate_trigger_activated_level(const LevelCondition& condition);
    
    ValidationResult validate_agent_has_item(const PuzzleCondition& condition);
    
    ValidationResult validate_agent_has_ability(const PuzzleCondition& condition);
    
    ValidationResult validate_door_state(const PuzzleCondition& condition);
    
    ValidationResult validate_water_level(const PuzzleCondition& condition);
    
    ValidationResult validate_switch_state(const PuzzleCondition& condition);
    
    ValidationResult validate_quantum_node_activated(const PuzzleCondition& condition);
    
    ValidationResult validate_multi_agent_coordination(const PuzzleCondition& condition);
    
    EntityID find_agent_by_number(uint8_t agent_number);
    
    EntityID find_entity_by_type(const std::string& entity_type, const std::unordered_map<std::string, std::string>& properties = {});
    
public:
    PuzzleValidator(EntityManager* entity_manager, ComponentRegistry* component_registry);
    
    ~PuzzleValidator() = default;
    
    PuzzleValidator(const PuzzleValidator&) = delete;
    PuzzleValidator& operator=(const PuzzleValidator&) = delete;
    
    PuzzleValidator(PuzzleValidator&&) noexcept = default;
    PuzzleValidator& operator=(PuzzleValidator&&) noexcept = default;

    ValidationResult validate_condition(const LevelCondition& condition);
    
    ValidationResult validate_condition(const PuzzleCondition& condition);

    std::vector<ValidationResult> validate_all_conditions(const std::vector<LevelCondition>& conditions);
    
    std::vector<ValidationResult> validate_all_conditions(const std::vector<PuzzleCondition>& conditions);

    bool are_all_conditions_met(const std::vector<LevelCondition>& conditions);
    
    bool are_all_conditions_met(const std::vector<PuzzleCondition>& conditions);

    float get_completion_percentage(const std::vector<LevelCondition>& conditions);
    
    float get_completion_percentage(const std::vector<PuzzleCondition>& conditions);

    std::vector<std::string> get_status_report(const std::vector<LevelCondition>& conditions);
    
    std::vector<std::string> get_status_report(const std::vector<PuzzleCondition>& conditions);
    
    void register_validator(const std::string& condition_type, std::function<ValidationResult(const PuzzleCondition&)> validator);
};