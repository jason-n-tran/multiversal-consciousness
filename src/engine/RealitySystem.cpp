#include "RealitySystem.h"
#include "HUDSystem.h"
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

    if (input_manager_ && input_manager_->is_action_just_pressed(InputAction::SWITCH_REALITY)) {
        save_current_reality_transforms();
        save_current_reality_inventories();
        save_current_reality_quantum_nodes();
        
        switch_reality();
        std::cout << "Switched to Reality " << (get_current_reality() == Reality::A ? "A" : "B") << std::endl;
        
        load_current_reality_transforms();
        load_current_reality_inventories();
        load_current_reality_quantum_nodes();
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

    bool success = reality_manager_->switch_reality();
    
    if (success && hud_system_) {
        hud_system_->on_reality_changed(reality_manager_->get_current_reality());
    }
    
    return success;
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
    
    bool is_unlinked_agent = false;
    if (component_registry_->has_component<Agent>(entity)) {
        const Agent* agent = component_registry_->get_component<Agent>(entity);
        if (agent) {
            if (!agent->position_linked) {
                is_unlinked_agent = true;
            }
        }
    }
    
    if (component_registry_->has_component<Transform>(entity)) {
        Transform* transform = component_registry_->get_component<Transform>(entity);
        if (transform) {
            if (is_unlinked_agent) {
                Reality current_reality = reality_manager_->get_current_reality();
                
                reality_manager_->set_reality_transform(entity, *transform, current_reality);
            } else {
                reality_manager_->sync_shared_geometry(entity, *transform);
            }
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

void RealitySystem::save_current_reality_transforms() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    Reality current_reality = reality_manager_->get_current_reality();
    
    for (EntityID agent_entity : agent_entities) {
        const Agent* agent = component_registry_->get_component<Agent>(agent_entity);
        if (agent && !agent->position_linked) {
            const Transform* transform = component_registry_->get_component<Transform>(agent_entity);
            if (transform) {
                reality_manager_->set_reality_transform(agent_entity, *transform, current_reality);
            }
        }
    }
}

void RealitySystem::load_current_reality_transforms() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& agent_entities = agent_container->get_entities();
    Reality current_reality = reality_manager_->get_current_reality();
    
    for (EntityID agent_entity : agent_entities) {
        const Agent* agent = component_registry_->get_component<Agent>(agent_entity);
        if (agent && !agent->position_linked) {
            const Transform* reality_transform = reality_manager_->get_reality_transform(agent_entity, current_reality);
            if (reality_transform) {
                Transform* current_transform = component_registry_->get_component<Transform>(agent_entity);
                if (current_transform) {
                    *current_transform = *reality_transform;
                }
            }
        }
    }
}

void RealitySystem::save_current_reality_inventories() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    Reality current_reality = reality_manager_->get_current_reality();
    const auto* inventory_container = component_registry_->get_all_components<Inventory>();
    
    if (inventory_container) {
        const auto& entities = inventory_container->get_entities();
        for (EntityID entity : entities) {
            const Inventory* inventory = component_registry_->get_component<Inventory>(entity);
            if (inventory) {
                reality_manager_->set_reality_inventory(entity, *inventory, current_reality);
            }
        }
    }
}

void RealitySystem::load_current_reality_inventories() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    Reality current_reality = reality_manager_->get_current_reality();
    const auto* inventory_container = component_registry_->get_all_components<Inventory>();
    
    if (inventory_container) {
        const auto& entities = inventory_container->get_entities();
        for (EntityID entity : entities) {
            const Inventory* reality_inventory = reality_manager_->get_reality_inventory(entity, current_reality);
            if (reality_inventory) {
                Inventory* current_inventory = component_registry_->get_component<Inventory>(entity);
                if (current_inventory) {
                    *current_inventory = *reality_inventory;
                }
            }
        }
    }
}

void RealitySystem::save_current_reality_quantum_nodes() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    Reality current_reality = reality_manager_->get_current_reality();
    const auto* node_container = component_registry_->get_all_components<QuantumNode>();
    
    if (node_container) {
        const auto& entities = node_container->get_entities();
        for (EntityID entity : entities) {
            const QuantumNode* node = component_registry_->get_component<QuantumNode>(entity);
            if (node) {
                reality_manager_->set_reality_quantum_node(entity, *node, current_reality);
            }
        }
    }
}

void RealitySystem::load_current_reality_quantum_nodes() {
    if (!entity_manager_ || !component_registry_ || !reality_manager_) {
        return;
    }
    
    Reality current_reality = reality_manager_->get_current_reality();
    const auto* node_container = component_registry_->get_all_components<QuantumNode>();
    
    if (node_container) {
        const auto& entities = node_container->get_entities();
        for (EntityID entity : entities) {
            const QuantumNode* reality_node = reality_manager_->get_reality_quantum_node(entity, current_reality);
            if (reality_node) {
                QuantumNode* current_node = component_registry_->get_component<QuantumNode>(entity);
                if (current_node) {
                    *current_node = *reality_node;
                }
            }
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

void RealitySystem::set_input_manager(InputManager* input_manager) {
    input_manager_ = input_manager;
}

void RealitySystem::set_hud_system(HUDSystem* hud_system) {
    hud_system_ = hud_system;
}

void RealitySystem::reset() {
    if (reality_manager_) {
        reality_manager_->reset();
    }
    std::cout << "Reality System reset" << std::endl;
}
