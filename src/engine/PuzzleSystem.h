#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "CoordinationSystem.h"
#include "Components.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <string>

struct PuzzleCondition {
    std::string condition_id;                               
    std::string condition_type;                             
    std::vector<EntityID> required_entities;               
    std::unordered_map<std::string, float> parameters;     
    std::function<bool()> check_function;                 
    bool is_met{false};                                   
    
    PuzzleCondition(const std::string& id, const std::string& type, 
                   const std::vector<EntityID>& entities, std::function<bool()> check_func)
        : condition_id(id)
        , condition_type(type)
        , required_entities(entities)
        , check_function(check_func) {}
};

struct Puzzle {
    std::string puzzle_id;                              
    std::string puzzle_name;                               
    std::vector<PuzzleCondition> conditions;              
    bool require_all_conditions{true};                    
    bool is_complete{false};                               
    std::chrono::steady_clock::time_point completion_time;  
    std::vector<EntityID> participating_agents;           
    
    Puzzle() = default;
    
    Puzzle(const std::string& id, const std::string& name, bool require_all = true)
        : puzzle_id(id)
        , puzzle_name(name)
        , require_all_conditions(require_all) {}
};

struct ActionFeedback {
    EntityID source_agent;                              
    std::vector<EntityID> affected_agents;                
    std::string feedback_type;                             
    std::string feedback_message;                           
    std::chrono::steady_clock::time_point timestamp;       
    float display_duration{3.0f};                         
    bool is_displayed{false};                              
    
    ActionFeedback(EntityID source, const std::vector<EntityID>& affected, 
                  const std::string& type, const std::string& message)
        : source_agent(source)
        , affected_agents(affected)
        , feedback_type(type)
        , feedback_message(message)
        , timestamp(std::chrono::steady_clock::now()) {}
};

class PuzzleSystem : public ISystem {
private:
    CoordinationSystem* coordination_system_{nullptr};      
    std::unordered_map<std::string, Puzzle> active_puzzles_; 
    std::vector<std::string> completed_puzzles_;            
    
    std::vector<ActionFeedback> active_feedback_;          
    std::unordered_map<std::string, std::function<void(const AgentEffect&)>> feedback_generators_; 
    std::unordered_map<std::string, std::function<bool(const PuzzleCondition&)>> condition_checkers_;
    
public:
    PuzzleSystem() = default;
    
    ~PuzzleSystem() override = default;
    
    PuzzleSystem(const PuzzleSystem&) = delete;
    PuzzleSystem& operator=(const PuzzleSystem&) = delete;
    
    PuzzleSystem(PuzzleSystem&&) noexcept = default;
    PuzzleSystem& operator=(PuzzleSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    void set_coordination_system(CoordinationSystem* coordination_system);
    
    void register_puzzle(const Puzzle& puzzle);
    
    bool add_puzzle_condition(const std::string& puzzle_id, const PuzzleCondition& condition);
    
    void check_puzzle_completion();
    
    bool is_puzzle_complete(const std::string& puzzle_id) const;
    
    const std::vector<std::string>& get_completed_puzzles() const {
        return completed_puzzles_;
    }
    
    void generate_action_feedback(EntityID source_agent, const std::vector<EntityID>& affected_agents, 
                                 const std::string& action_type);
    
    void update_feedback_display(float delta_time);
    
    const std::vector<ActionFeedback>& get_active_feedback() const {
        return active_feedback_;
    }
    
    PuzzleCondition create_position_condition(const std::string& condition_id,
                                             const std::vector<EntityID>& agents,
                                             const std::vector<std::pair<float, float>>& target_positions,
                                             float tolerance = 16.0f);
    
    PuzzleCondition create_inventory_condition(const std::string& condition_id,
                                              const std::vector<EntityID>& agents,
                                              const std::vector<std::string>& required_items);
    
    PuzzleCondition create_environmental_condition(const std::string& condition_id,
                                                  const std::vector<EntityID>& env_entities,
                                                  const std::vector<std::string>& required_states);
    
    PuzzleCondition create_timing_condition(const std::string& condition_id,
                                           const std::vector<EntityID>& agents,
                                           std::chrono::milliseconds time_window,
                                           const std::string& action_type);
    
private:
    void initialize_condition_checkers();
    
    void initialize_feedback_generators();
    
    bool check_position_condition(const PuzzleCondition& condition);
    
    bool check_inventory_condition(const PuzzleCondition& condition);
    
    bool check_environmental_condition(const PuzzleCondition& condition);
    
    bool check_timing_condition(const PuzzleCondition& condition);
    
    void generate_movement_feedback(const AgentEffect& effect);
    
    void generate_environmental_feedback(const AgentEffect& effect);
    
    void generate_item_feedback(const AgentEffect& effect);
    
    float calculate_distance(EntityID entity1, EntityID entity2) const;
};