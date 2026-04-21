#pragma once

#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class PuzzleValidator;

struct LevelAgent {
    std::string agent_type;   
    uint8_t agent_number;      
    float x, y;                 
    float movement_speed{100.0f}; 
    std::unordered_map<std::string, std::string> initial_abilities; 
};

struct LevelQuantumNode {
    float x, y;                
    std::string reality_a_item; 
    std::string reality_b_item; 
    float interaction_radius{32.0f}; 
};

struct LevelEnvironment {
    std::string type;           
    float x, y;                 
    std::unordered_map<std::string, std::string> properties;
};

struct PuzzleCondition {
    std::string type;           
    std::string target;          
    std::string value;          
};

struct LevelData {
    std::string name;
    std::string description;
    std::vector<LevelAgent> agents;
    std::vector<LevelQuantumNode> quantum_nodes;
    std::vector<LevelEnvironment> environment;
    std::vector<PuzzleCondition> completion_conditions;
    
    float width{1280.0f};
    float height{720.0f};
};

class LevelLoader {
private:
    EntityManager* entity_manager_{nullptr};
    ComponentRegistry* component_registry_{nullptr};
    std::unique_ptr<PuzzleValidator> puzzle_validator_;
    
    std::vector<LevelAgent> parse_agents(const std::string& data);
    
    std::vector<LevelQuantumNode> parse_quantum_nodes(const std::string& data);
    
    std::vector<LevelEnvironment> parse_environment(const std::string& data);
    
    std::vector<PuzzleCondition> parse_conditions(const std::string& data);
    
public:
    LevelLoader(EntityManager* entity_manager, ComponentRegistry* component_registry);
    
    ~LevelLoader() = default;
    
    LevelLoader(const LevelLoader&) = delete;
    LevelLoader& operator=(const LevelLoader&) = delete;
    
    LevelLoader(LevelLoader&&) noexcept = default;
    LevelLoader& operator=(LevelLoader&&) noexcept = default;
    
    LevelData load_level_from_file(const std::string& filename);
    
    std::vector<EntityID> instantiate_level(const LevelData& level_data);
    
    EntityID create_agent(const LevelAgent& agent_data);
    
    EntityID create_quantum_node(const LevelQuantumNode& node_data);
    
    EntityID create_environment(const LevelEnvironment& env_data);
    
    bool validate_puzzle_completion(const std::vector<PuzzleCondition>& conditions);
    
    bool check_condition(const PuzzleCondition& condition);
    
    PuzzleValidator& get_puzzle_validator() { return *puzzle_validator_; }
    
    const PuzzleValidator& get_puzzle_validator() const { return *puzzle_validator_; }
};