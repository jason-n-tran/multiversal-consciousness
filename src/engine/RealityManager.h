#pragma once

#include <cstdint>
#include <array>
#include <unordered_map>
#include <chrono>
#include "EntityManager.h"
#include "Components.h"

enum class Reality : uint8_t {
    A = 0,
    B = 1
};


class RealityManager {
private:
    Reality current_reality_{Reality::A};
    
    std::array<std::unordered_map<EntityID, Inventory>, 2> reality_inventories_;
    std::array<std::unordered_map<EntityID, QuantumNode>, 2> reality_quantum_nodes_;
    
    std::unordered_map<EntityID, Transform> shared_geometry_;
    
    std::unordered_map<EntityID, Door> shared_doors_;
    std::unordered_map<EntityID, WaterLevel> shared_water_levels_;
    std::unordered_map<EntityID, EnvironmentalSwitch> shared_switches_;
    
    std::chrono::steady_clock::time_point last_switch_time_;
    static constexpr std::chrono::milliseconds MAX_SWITCH_TIME{100};
    
public:
    RealityManager();
    
    ~RealityManager() = default;
    
    RealityManager(const RealityManager&) = delete;
    RealityManager& operator=(const RealityManager&) = delete;
    
    RealityManager(RealityManager&&) noexcept = default;
    RealityManager& operator=(RealityManager&&) noexcept = default;
    
    bool switch_reality();
    
    Reality get_current_reality() const { return current_reality_; }
    
    void set_current_reality(Reality reality) { current_reality_ = reality; }
    
    void sync_shared_geometry(EntityID entity, const Transform& transform);
    
    const Transform* get_shared_geometry(EntityID entity) const;
    
    const Inventory* get_reality_inventory(EntityID entity, Reality reality = Reality::A) const;
    
    void set_reality_inventory(EntityID entity, const Inventory& inventory, Reality reality = Reality::A);
    
    const QuantumNode* get_reality_quantum_node(EntityID entity, Reality reality = Reality::A) const;
    
    void set_reality_quantum_node(EntityID entity, const QuantumNode& quantum_node, Reality reality = Reality::A);
    
    void sync_shared_door(EntityID entity, const Door& door);
    
    const Door* get_shared_door(EntityID entity) const;
    
    void sync_shared_water_level(EntityID entity, const WaterLevel& water_level);
    
    const WaterLevel* get_shared_water_level(EntityID entity) const;
    
    void sync_shared_switch(EntityID entity, const EnvironmentalSwitch& env_switch);
    
    const EnvironmentalSwitch* get_shared_switch(EntityID entity) const;
    
    void remove_entity(EntityID entity);
    
    std::chrono::milliseconds get_time_since_last_switch() const;
    
    bool last_switch_within_performance_limit() const;
};