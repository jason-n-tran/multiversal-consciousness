#include "RealitySystem.h"
#include "ComponentRegistry.h"
#include "EntityManager.h"
#include <iostream>

RealitySystem::RealitySystem() 
    : reality_manager_(std::make_unique<RealityManager>()) {
}

void RealitySystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    entity_manager_ = &entity_manager;
    component_registry_ = &component_registry;
    
    std::cout << "Reality System initialized" << std::endl;
}

void RealitySystem::update(float delta_time) {
    (void)delta_time;
    
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    const auto* transform_container = component_registry_->get_all_components<Transform>();
    if (transform_container) {
        const auto& entities_with_transform = transform_container->get_entities();
        
        for (EntityID entity : entities_with_transform) {
            if (entity_manager_->is_valid(entity)) {
                synchronize_entity(entity);
            }
        }
    }
}

void RealitySystem::shutdown() {
    std::cout << "Reality System shutdown" << std::endl;
}

bool RealitySystem::switch_reality() {
    if (!reality_manager_) {
        return false;
    }
    
    return reality_manager_->switch_reality();
}

Reality RealitySystem::get_current_reality() const {
    if (!reality_manager_) {
        return Reality::A; 
    }
    
    return reality_manager_->get_current_reality();
}

void RealitySystem::synchronize_entity(EntityID entity) {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    if (component_registry_->has_component<Transform>(entity)) {
        const Transform* transform = component_registry_->get_component<Transform>(entity);
        if (transform) {
            reality_manager_->sync_shared_geometry(entity, *transform);
        }
    }
    
    if (component_registry_->has_component<Inventory>(entity)) {
        const Inventory* inventory = component_registry_->get_component<Inventory>(entity);
        if (inventory) {
            reality_manager_->set_reality_inventory(entity, *inventory, reality_manager_->get_current_reality());
        }
    }
    
    if (component_registry_->has_component<QuantumNode>(entity)) {
        const QuantumNode* quantum_node = component_registry_->get_component<QuantumNode>(entity);
        if (quantum_node) {
            reality_manager_->set_reality_quantum_node(entity, *quantum_node, reality_manager_->get_current_reality());
        }
    }

    if (component_registry_->has_component<Door>(entity)) {
        const Door* door = component_registry_->get_component<Door>(entity);
        if (door) {
            reality_manager_->sync_shared_door(entity, *door);
        }
    }
    
    if (component_registry_->has_component<WaterLevel>(entity)) {
        const WaterLevel* water_level = component_registry_->get_component<WaterLevel>(entity);
        if (water_level) {
            reality_manager_->sync_shared_water_level(entity, *water_level);
        }
    }
    
    if (component_registry_->has_component<EnvironmentalSwitch>(entity)) {
        const EnvironmentalSwitch* env_switch = component_registry_->get_component<EnvironmentalSwitch>(entity);
        if (env_switch) {
            reality_manager_->sync_shared_switch(entity, *env_switch);
        }
    }
}

void RealitySystem::handle_entity_destruction(EntityID entity) {
    if (reality_manager_) {
        reality_manager_->remove_entity(entity);
    }
}


const Door* RealitySystem::get_shared_door(EntityID entity) const {
    if (!reality_manager_) {
        return nullptr;
    }
    return reality_manager_->get_shared_door(entity);
}

const WaterLevel* RealitySystem::get_shared_water_level(EntityID entity) const {
    if (!reality_manager_) {
        return nullptr;
    }
    return reality_manager_->get_shared_water_level(entity);
}

const EnvironmentalSwitch* RealitySystem::get_shared_switch(EntityID entity) const {
    if (!reality_manager_) {
        return nullptr;
    }
    return reality_manager_->get_shared_switch(entity);
}