#pragma once

#include "System.h"
#include "RealityManager.h"
#include "InputManager.h"
#include <memory>


class RealitySystem : public ISystem {
private:
    std::unique_ptr<RealityManager> reality_manager_;
    InputManager* input_manager_{nullptr};
    
public:

    RealitySystem();

    ~RealitySystem() override = default;
    
    RealitySystem(const RealitySystem&) = delete;
    RealitySystem& operator=(const RealitySystem&) = delete;
    
    RealitySystem(RealitySystem&&) noexcept = default;
    RealitySystem& operator=(RealitySystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;

    RealityManager& get_reality_manager() { return *reality_manager_; }
    
    const RealityManager& get_reality_manager() const { return *reality_manager_; }
    
    bool switch_reality();
    
    Reality get_current_reality() const;
    
    void synchronize_entity(EntityID entity);
    
    void handle_entity_destruction(EntityID entity);
    
    void set_input_manager(InputManager* input_manager);
    
    const Door* get_shared_door(EntityID entity) const;
    
    const WaterLevel* get_shared_water_level(EntityID entity) const;
    
    const EnvironmentalSwitch* get_shared_switch(EntityID entity) const;
};