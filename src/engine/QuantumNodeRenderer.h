#pragma once

#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include "CameraController.h"
#include "RealityManager.h"
#include <SDL3/SDL.h>
#include <memory>


struct QuantumNodeVisualConfig {
    SDL_FColor reality_a_color{0.2f, 0.6f, 1.0f, 0.8f};         
    SDL_FColor reality_a_glow_color{0.2f, 0.6f, 1.0f, 0.4f};   
    
    SDL_FColor reality_b_color{1.0f, 0.4f, 0.2f, 0.8f};        
    SDL_FColor reality_b_glow_color{1.0f, 0.4f, 0.2f, 0.4f};
    
    SDL_FColor activated_color{0.0f, 1.0f, 0.0f, 1.0f};       
    SDL_FColor activated_glow_color{0.0f, 1.0f, 0.0f, 0.6f};   
    
    float glow_radius{16.0f};                             
    float pulse_speed{3.0f};                               
    float pulse_min{0.3f};                                       
    float pulse_max{1.0f};                                      
    
    SDL_FColor prompt_background_color{0.0f, 0.0f, 0.0f, 0.8f};
    SDL_FColor prompt_text_color{1.0f, 1.0f, 1.0f, 1.0f};      
    float prompt_offset_y{-40.0f};                             
};

class QuantumNodeRenderer : public IRenderSystem {
private:
    QuantumNodeVisualConfig visual_config_;                  
    CameraController* camera_controller_{nullptr};          
    RealityManager* reality_manager_{nullptr};               
    float animation_time_{0.0f};                          
    
    void render_quantum_node(SDL_Renderer* renderer, EntityID entity,
                            const Transform& transform, const QuantumNode& quantum_node,
                            Reality current_reality);
    
    void render_glow_effect(SDL_Renderer* renderer, float x, float y, 
                           float radius, const SDL_FColor& color, float intensity);
    
    void render_interaction_prompt(SDL_Renderer* renderer, float x, float y,
                                  const QuantumNode& quantum_node, Reality current_reality);
    
    float calculate_pulse_intensity() const;
    
    SDL_FColor get_quantum_node_color(const QuantumNode& quantum_node, Reality current_reality) const;
    
    SDL_FColor get_quantum_node_glow_color(const QuantumNode& quantum_node, Reality current_reality) const;
    
public:
    QuantumNodeRenderer() = default;
    
    ~QuantumNodeRenderer() override = default;
    
    QuantumNodeRenderer(const QuantumNodeRenderer&) = delete;
    QuantumNodeRenderer& operator=(const QuantumNodeRenderer&) = delete;
    
    QuantumNodeRenderer(QuantumNodeRenderer&&) noexcept = default;
    QuantumNodeRenderer& operator=(QuantumNodeRenderer&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void render(SDL_Renderer* renderer) override;
    
    void shutdown() override;
    
    void set_camera_controller(CameraController* camera_controller) { camera_controller_ = camera_controller; }
    
    void set_reality_manager(RealityManager* reality_manager) { reality_manager_ = reality_manager; }
    
    QuantumNodeVisualConfig& get_visual_config() { return visual_config_; }
    
    const QuantumNodeVisualConfig& get_visual_config() const { return visual_config_; }
    
    void set_visual_config(const QuantumNodeVisualConfig& config) { visual_config_ = config; }
};