#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "RealityManager.h"
#include "Components.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <functional>
#include <memory>

struct AgentAction {
    EntityID agent_entity;                           
    std::string action_type;                                
    Reality action_reality;                                  
    std::chrono::steady_clock::time_point timestamp;       
    std::unordered_map<std::string, float> parameters;     
    
    AgentAction(EntityID agent, const std::string& type, Reality reality)
        : agent_entity(agent)
        , action_type(type)
        , action_reality(reality)
        , timestamp(std::chrono::steady_clock::now()) {}
};

struct AgentEffect {
    EntityID source_agent;                                  
    EntityID target_agent;                                  
    std::string effect_type;                                
    Reality effect_reality;                                 
    std::chrono::steady_clock::time_point timestamp;        
    std::unordered_map<std::string, float> effect_data;     
    
    AgentEffect(EntityID source, EntityID target, const std::string& type, Reality reality)
        : source_agent(source)
        , target_agent(target)
        , effect_type(type)
        , effect_reality(reality)
        , timestamp(std::chrono::steady_clock::now()) {}
};

struct TimingSensitiveMechanic {
    std::string mechanic_id;                                
    std::vector<EntityID> required_agents;                  
    std::chrono::milliseconds time_window;              
    std::chrono::steady_clock::time_point start_time;     
    bool is_active{false};                                 
    std::unordered_set<EntityID> participating_agents;     
    std::function<bool()> completion_check;                
    
    TimingSensitiveMechanic() = default;
    
    TimingSensitiveMechanic(const std::string& id, 
                           const std::vector<EntityID>& agents,
                           std::chrono::milliseconds window,
                           std::function<bool()> check_func)
        : mechanic_id(id)
        , required_agents(agents)
        , time_window(window)
        , completion_check(check_func) {}
};

class CoordinationSystem : public ISystem {
private:
    std::unique_ptr<RealityManager> reality_manager_;       
    
    std::vector<AgentAction> recent_actions_;                
    std::vector<AgentEffect> pending_effects_;           
    static constexpr size_t MAX_ACTION_HISTORY = 100;      
    
    std::unordered_map<std::string, TimingSensitiveMechanic> active_mechanics_; 
    std::unordered_map<std::string, std::vector<EntityID>> puzzle_participants_; 
    std::unordered_map<std::string, bool> puzzle_completion_status_;            
    
    std::unordered_map<std::string, std::function<void(const AgentAction&)>> effect_evaluators_;
    
public:
    explicit CoordinationSystem(std::unique_ptr<RealityManager> reality_manager);
    
    ~CoordinationSystem() override = default;
    
    CoordinationSystem(const CoordinationSystem&) = delete;
    CoordinationSystem& operator=(const CoordinationSystem&) = delete;
    
    CoordinationSystem(CoordinationSystem&&) noexcept = default;
    CoordinationSystem& operator=(CoordinationSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    void record_agent_action(EntityID agent_entity, const std::string& action_type, 
                           Reality reality, const std::unordered_map<std::string, float>& parameters = {});
    
    void evaluate_inter_agent_effects();
    
    void apply_pending_effects();
    
    void register_puzzle_scenario(const std::string& puzzle_id, const std::vector<EntityID>& participating_agents);
    
    bool is_puzzle_complete(const std::string& puzzle_id) const;
    
    bool start_timing_mechanic(const std::string& mechanic_id,
                              const std::vector<EntityID>& required_agents,
                              std::chrono::milliseconds time_window,
                              std::function<bool()> completion_check);
    
    void update_timing_mechanics(float delta_time);
    
    void handle_reality_switch();
    
    const std::unordered_map<std::string, TimingSensitiveMechanic>& get_active_mechanics() const {
        return active_mechanics_;
    }
    
    const std::vector<AgentAction>& get_recent_actions() const {
        return recent_actions_;
    }
    
    const std::vector<AgentEffect>& get_pending_effects() const {
        return pending_effects_;
    }
    RealityManager& get_reality_manager() {
        return *reality_manager_;
    }
    const RealityManager& get_reality_manager() const {
        return *reality_manager_;
    }
    
private:
    void initialize_effect_evaluators();
    
    void evaluate_movement_effects(const AgentAction& action);
    
    void evaluate_activation_effects(const AgentAction& action);
    
    void evaluate_quantum_effects(const AgentAction& action);
    
    void cleanup_action_history();
    
    bool are_agents_in_range(EntityID agent1, EntityID agent2, float max_distance) const;
    
    float calculate_distance(EntityID entity1, EntityID entity2) const;
};