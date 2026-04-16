#include "RealityManager.h"
#include <iostream>

RealityManager::RealityManager() 
    : current_reality_(Reality::A)
    , last_switch_time_(std::chrono::steady_clock::now()) {
}

bool RealityManager::switch_reality() {
    auto switch_start = std::chrono::steady_clock::now();
    
    current_reality_ = (current_reality_ == Reality::A) ? Reality::B : Reality::A;
    
    auto switch_end = std::chrono::steady_clock::now();
    auto switch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(switch_end - switch_start);
    
    last_switch_time_ = switch_end;
    
    bool within_limit = switch_duration <= MAX_SWITCH_TIME;
    
    if (!within_limit) {
        std::cerr << "Reality switch took " << switch_duration.count() 
                  << "ms, exceeding limit of " << MAX_SWITCH_TIME.count() << "ms" << std::endl;
    }
    
    return within_limit;
}

void RealityManager::sync_shared_geometry(EntityID entity, const Transform& transform) {
    shared_geometry_[entity] = transform;
}

const Transform* RealityManager::get_shared_geometry(EntityID entity) const {
    auto it = shared_geometry_.find(entity);
    return (it != shared_geometry_.end()) ? &it->second : nullptr;
}

const Inventory* RealityManager::get_reality_inventory(EntityID entity, Reality reality) const {
    const auto& reality_map = reality_inventories_[static_cast<size_t>(reality)];
    auto it = reality_map.find(entity);
    return (it != reality_map.end()) ? &it->second : nullptr;
}

void RealityManager::set_reality_inventory(EntityID entity, const Inventory& inventory, Reality reality) {
    reality_inventories_[static_cast<size_t>(reality)][entity] = inventory;
}

const QuantumNode* RealityManager::get_reality_quantum_node(EntityID entity, Reality reality) const {
    const auto& reality_map = reality_quantum_nodes_[static_cast<size_t>(reality)];
    auto it = reality_map.find(entity);
    return (it != reality_map.end()) ? &it->second : nullptr;
}

void RealityManager::set_reality_quantum_node(EntityID entity, const QuantumNode& quantum_node, Reality reality) {
    reality_quantum_nodes_[static_cast<size_t>(reality)][entity] = quantum_node;
}


void RealityManager::sync_shared_door(EntityID entity, const Door& door) {
    shared_doors_[entity] = door;
}

const Door* RealityManager::get_shared_door(EntityID entity) const {
    auto it = shared_doors_.find(entity);
    return (it != shared_doors_.end()) ? &it->second : nullptr;
}

void RealityManager::sync_shared_water_level(EntityID entity, const WaterLevel& water_level) {
    shared_water_levels_[entity] = water_level;
}

const WaterLevel* RealityManager::get_shared_water_level(EntityID entity) const {
    auto it = shared_water_levels_.find(entity);
    return (it != shared_water_levels_.end()) ? &it->second : nullptr;
}

void RealityManager::sync_shared_switch(EntityID entity, const EnvironmentalSwitch& env_switch) {
    shared_switches_[entity] = env_switch;
}

const EnvironmentalSwitch* RealityManager::get_shared_switch(EntityID entity) const {
    auto it = shared_switches_.find(entity);
    return (it != shared_switches_.end()) ? &it->second : nullptr;
}

void RealityManager::remove_entity(EntityID entity) {
    shared_geometry_.erase(entity);
    
    reality_inventories_[0].erase(entity);
    reality_inventories_[1].erase(entity);
    
    reality_quantum_nodes_[0].erase(entity);
    reality_quantum_nodes_[1].erase(entity);
    
    shared_doors_.erase(entity);
    shared_water_levels_.erase(entity);
    shared_switches_.erase(entity);
}

std::chrono::milliseconds RealityManager::get_time_since_last_switch() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - last_switch_time_);
}

bool RealityManager::last_switch_within_performance_limit() const {
    return get_time_since_last_switch() <= MAX_SWITCH_TIME;
}