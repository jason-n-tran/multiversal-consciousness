#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "RealityManager.h"
#include "Components.h"
#include "InputManager.h"
#include <queue>
#include <chrono>
#include <memory>
#include <unordered_map>

class PossessionSystem;

struct QuantumInteraction {
    EntityID node_entity;                             
    EntityID agent_entity;                              
    Reality trigger_reality;                                
    std::chrono::steady_clock::time_point timestamp;       
    
    QuantumInteraction(EntityID node, EntityID agent, Reality reality)
        : node_entity(node)
        , agent_entity(agent)
        , trigger_reality(reality)
        , timestamp(std::chrono::steady_clock::now()) {}
};

class QuantumSystem : public ISystem {
private:
    std::queue<QuantumInteraction> pending_interactions_;    
    std::unique_ptr<RealityManager> reality_manager_; 
    InputManager* input_manager_{nullptr}; 
    PossessionSystem* possession_system_{nullptr};
    
    static constexpr float DEFAULT_INTERACTION_RADIUS = 32.0f;
    
    std::unordered_map<EntityID, EntityID> agent_prompt_targets_; 
    
public:
    explicit QuantumSystem(std::unique_ptr<RealityManager> reality_manager);
    
    ~QuantumSystem() override = default;
    
    QuantumSystem(const QuantumSystem&) = delete;
    QuantumSystem& operator=(const QuantumSystem&) = delete;
    
    QuantumSystem(QuantumSystem&&) noexcept = default;
    QuantumSystem& operator=(QuantumSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void update_interaction_prompts(float delta_time);
    
    void show_interaction_prompt(EntityID agent_entity, EntityID node_entity);
    
    void hide_interaction_prompt(EntityID agent_entity);
    
    void shutdown() override;
    
    bool trigger_quantum_node(EntityID node_entity, EntityID agent_entity);
    
    bool is_agent_in_range(EntityID node_entity, EntityID agent_entity) const;
    
    std::vector<EntityID> get_nearby_quantum_nodes(EntityID agent_entity) const;
    
    void process_quantum_distribution(const QuantumInteraction& interaction);
    
    const RealityManager& get_reality_manager() const {
        return *reality_manager_;
    }
    
    RealityManager& get_reality_manager() {
        return *reality_manager_;
    }
    
    size_t get_pending_interaction_count() const {
        return pending_interactions_.size();
    }

    void set_input_manager(InputManager* input_manager);
    
    void set_possession_system(PossessionSystem* possession_system);
    
private:
    void trigger_interaction_for_agent(EntityID agent_entity);

    float calculate_distance(EntityID entity1, EntityID entity2) const;
    
    bool validate_entity_components(EntityID entity, bool require_agent) const;
    
    void add_item_to_agent(EntityID agent_entity, const std::string& item_name, Reality reality);
    
    void activate_quantum_node(EntityID node_entity);
};