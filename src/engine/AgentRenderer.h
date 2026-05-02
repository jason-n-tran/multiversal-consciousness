#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "CameraController.h"
#include "RealityManager.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <memory>
#include <unordered_map>
#include <string>

enum class AnimationState {
    Idle,
    Run,
    Jump,
    Fall
};

struct SpriteAnimation {
    SDL_Texture* texture{nullptr};
    int frame_count{0};
    int frame_width{0};
    int frame_height{0};
    float frame_duration{0.1f};  
};

struct AgentSpriteData {
    SpriteAnimation idle;
    SpriteAnimation run;
    int sprite_width{0};
    int sprite_height{0};
};

struct AgentVisualConfig {
    SDL_FColor possessed_outline_color{0.0f, 1.0f, 0.0f, 1.0f};  
    SDL_FColor possessed_glow_color{0.0f, 1.0f, 0.0f, 0.3f};    
    float possessed_outline_width{2.0f};                   
    float possessed_glow_radius{8.0f};                         
    
    SDL_FColor idle_outline_color{0.5f, 0.5f, 0.5f, 0.8f};     
    float idle_outline_width{1.0f};                            
    
    SDL_FColor number_text_color{1.0f, 1.0f, 1.0f, 1.0f};      
    SDL_FColor number_background_color{0.0f, 0.0f, 0.0f, 0.7f};  
    float number_offset_y{-40.0f};                               
    
    float glow_pulse_speed{2.0f};                            
    float glow_pulse_min{0.2f};                                 
    float glow_pulse_max{0.6f};                                
};

class AgentRenderer : public IRenderSystem {
private:
    AgentVisualConfig visual_config_;                        
    CameraController* camera_controller_{nullptr};    
    RealityManager* reality_manager_{nullptr};       
    float animation_time_{0.0f};                             
    SDL_Renderer* sdl_renderer_{nullptr};                        
    
    std::unordered_map<uint8_t, AgentSpriteData> agent_sprites_;
    
    std::unordered_map<EntityID, AnimationState> entity_animation_states_;
    std::unordered_map<EntityID, float> entity_animation_times_;
    std::unordered_map<EntityID, bool> entity_facing_right_;  
    void render_agent_feedback(SDL_Renderer* renderer, EntityID entity, 
                              const Transform& transform, const Agent& agent, 
                              const Renderable* renderable);
    
    void render_agent_with_animation(SDL_Renderer* renderer, EntityID entity,
                                    const Transform& transform, const Agent& agent,
                                    const Renderable& renderable, Reality current_reality);
    
    void render_simple_agent(SDL_Renderer* renderer, EntityID entity,
                            const Transform& transform, const Agent& agent,
                            const Renderable& renderable);
    
    SDL_Rect get_animation_frame(const Agent& agent, Reality current_reality, float animation_time) const;
    
    bool load_agent_sprites(uint8_t agent_number, const std::string& base_path);

    AnimationState get_animation_state(EntityID entity, const PhysicsComponent* physics) const;

    void update_entity_animation(EntityID entity, AnimationState new_state, float delta_time);

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
    
    void initialize_sprites(SDL_Renderer* renderer);
    
    void update(float delta_time) override;
    
    void render(SDL_Renderer* renderer) override;
    
    void shutdown() override;
    
    void set_camera_controller(CameraController* camera_controller);

    void set_reality_manager(RealityManager* reality_manager) { reality_manager_ = reality_manager; }
    
    AgentVisualConfig& get_visual_config() { return visual_config_; }
    
    const AgentVisualConfig& get_visual_config() const { return visual_config_; }
    
    void set_visual_config(const AgentVisualConfig& config);
};