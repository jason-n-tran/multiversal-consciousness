#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include <string>
#include <vector>

class VerificationSystem : public ISystem {
private:
    EntityManager* entity_manager_{nullptr};
    ComponentRegistry* component_registry_{nullptr};
    
    bool scenario_completed_{false};
    bool scenario_failed_{false};
    std::string feedback_message_;
    float feedback_timer_{0.0f};
    
    bool agent_reached_success_zone_{false};
    bool agent_has_required_ability_{false};
    bool correct_reality_active_{false};
    
    bool check_success_zone();
    
    bool check_required_ability();
    
    bool check_reality_state();
    
    void update_feedback();
    
    void reset_scenario();
    
public:
    VerificationSystem(EntityManager* entity_manager, ComponentRegistry* component_registry);
    
    ~VerificationSystem() override = default;
    
    VerificationSystem(const VerificationSystem&) = delete;
    VerificationSystem& operator=(const VerificationSystem&) = delete;
    
    VerificationSystem(VerificationSystem&&) noexcept = default;
    VerificationSystem& operator=(VerificationSystem&&) noexcept = default;
    
    void update(float delta_time) override;
    const std::string& get_feedback_message() const { return feedback_message_; }
    
    bool is_scenario_completed() const { return scenario_completed_; }
    
    bool is_scenario_failed() const { return scenario_failed_; }
    
    float get_feedback_timer() const { return feedback_timer_; }
    
    void trigger_reset() { reset_scenario(); }
    
    std::string get_verification_stats() const;
};