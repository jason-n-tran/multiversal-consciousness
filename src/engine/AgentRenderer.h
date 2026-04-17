#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "CameraController.h"
#include <SDL3/SDL.h>
#include <memory>

struct AgentVisualConfig {
    SDL_FColor possessed_outline_color{0.0f, 1.0f, 0.0f, 1.0f};  
    SDL_FColor possessed_glow_color{0.0f, 1.0f, 0.0f, 0.3f};    
    float possessed_outline_width{2.0f};                   
    float possessed_glow_radius{8.0f};                         
    
    SDL_FColor idle_outline_color{0.5f, 0.5f, 0.5f, 0.8f};     
    float idle_outline_width{1.0f};                            
    
    SDL_FColor number_text_color{1.0f, 1.0f, 1.0f, 1.0f};      
    SDL_FColor number_background_color{0.0f, 0.0f, 0.0f, 0.7f};  
    float number_offset_y{-20.0f};                               
    
    float glow_pulse_speed{2.0f};                            
    float glow_pulse_min{0.2f};                                 
    float glow_pulse_max{0.6f};                                
};

class AgentRenderer : public IRenderSystem {
private:
    AgentVisualConfig visual_config_;                        
    CameraController* camera_controller_{nullptr};           
    float animation_time_{0.0f};                             
    
    void render_agent_feedback(SDL_Renderer* renderer, EntityID entity, 
                              const Transform& transform, const Agent& agent, 
                              const Renderable* renderable);
    
    void render_outline(SDL_Renderer* renderer, float x, float y, 
                       float width, float height, 
                       const SDL_FColor& color, float line_width);
    
    void render_glow(SDL_Renderer* renderer, float x, float y, 
                    float width, float height, 
                    const SDL_FColor& color, float radius, float intensity);
    
    void render_agent_number(SDL_Renderer* renderer, float x, float y, uint8_t agent_number);
    
    float calculate_glow_intensity() const;
    
public:
    AgentRenderer() = default;
    
    ~AgentRenderer() override = default;
    
    AgentRenderer(const AgentRenderer&) = delete;
    AgentRenderer& operator=(const AgentRenderer&) = delete;
    
    AgentRenderer(AgentRenderer&&) noexcept = default;
    AgentRenderer& operator=(AgentRenderer&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void render(SDL_Renderer* renderer) override;
    
    void shutdown() override;
    
    void set_camera_controller(CameraController* camera_controller);
    
    AgentVisualConfig& get_visual_config() { return visual_config_; }
    
    const AgentVisualConfig& get_visual_config() const { return visual_config_; }
    
    void set_visual_config(const AgentVisualConfig& config);
};