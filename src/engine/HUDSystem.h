#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "PossessionSystem.h"
#include "QuantumLoadoutSystem.h"
#include "RealityManager.h"
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>

struct HUDVisualConfig {
    SDL_FColor agent_number_color{1.0f, 1.0f, 1.0f, 1.0f};       
    SDL_FColor agent_number_bg_color{0.0f, 0.0f, 0.0f, 0.8f};     
    float agent_number_font_size{24.0f};                         
    float agent_number_x{20.0f};                                 
    float agent_number_y{20.0f};                                 
    
    SDL_FColor ability_text_color{0.8f, 0.8f, 1.0f, 1.0f};      
    SDL_FColor ability_bg_color{0.0f, 0.0f, 0.2f, 0.8f};      
    float ability_font_size{18.0f};                              
    float ability_x{20.0f};                                      
    float ability_y{60.0f};                                   
    float ability_spacing{25.0f};                              
    
    float panel_padding{10.0f};                              
    float panel_corner_radius{5.0f};                           
    
    float fade_in_time{0.3f};                                 
    float fade_out_time{0.2f};                                  
    float update_animation_time{0.1f};                            
};

class HUDSystem : public IRenderSystem {
private:
    HUDVisualConfig visual_config_;                             
    PossessionSystem* possession_system_{nullptr};              
    QuantumLoadoutSystem* loadout_system_{nullptr};             
    RealityManager* reality_manager_{nullptr};                 
    
    uint8_t current_agent_number_{0};                         
    std::vector<std::string> current_abilities_;             
    Reality current_reality_{Reality::A};                   
    
    float agent_number_alpha_{1.0f};                         
    float ability_alpha_{1.0f};                              
    float update_animation_timer_{0.0f};                   
    bool needs_update_{true};                                
    
    void update_display_state();
    
    void render_agent_number(SDL_Renderer* renderer);
    
    void render_abilities(SDL_Renderer* renderer);
    
    void render_panel(SDL_Renderer* renderer, float x, float y, 
                     float width, float height, const SDL_FColor& color);
    
    float render_text_with_background(SDL_Renderer* renderer, const std::string& text,
                                     float x, float y, float font_size,
                                     const SDL_FColor& text_color, const SDL_FColor& bg_color);
    
    std::string get_ability_display_name(AbilityType ability) const;
    
    bool has_state_changed() const;
    
    void update_animations(float delta_time);
    
public:
    HUDSystem() = default;
    
    ~HUDSystem() override = default;
    
    HUDSystem(const HUDSystem&) = delete;
    HUDSystem& operator=(const HUDSystem&) = delete;
    
    HUDSystem(HUDSystem&&) noexcept = default;
    HUDSystem& operator=(HUDSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void render(SDL_Renderer* renderer) override;
    
    void shutdown() override;
    
    void set_possession_system(PossessionSystem* possession_system);
    
    void set_loadout_system(QuantumLoadoutSystem* loadout_system);
    
    void set_reality_manager(RealityManager* reality_manager);
    
    HUDVisualConfig& get_visual_config() { return visual_config_; }
    
    const HUDVisualConfig& get_visual_config() const { return visual_config_; }
    
    void set_visual_config(const HUDVisualConfig& config);
    
    void force_update() { needs_update_ = true; }
    
    void on_possession_changed(uint8_t new_agent_number);
    
    void on_reality_changed(Reality new_reality);
    
    void on_abilities_changed(EntityID agent_entity);
    
    uint8_t get_displayed_agent_number() const { return current_agent_number_; }
    
    const std::vector<std::string>& get_displayed_abilities() const { return current_abilities_; }
};