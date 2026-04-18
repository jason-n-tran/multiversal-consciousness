#include "QuantumNodeRenderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void QuantumNodeRenderer::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    entity_manager_ = &entity_manager;
    component_registry_ = &component_registry;
    std::cout << "QuantumNodeRenderer initialized" << std::endl;
}

void QuantumNodeRenderer::update(float delta_time) {
    animation_time_ += delta_time;
}

void QuantumNodeRenderer::render(SDL_Renderer* renderer) {
    if (!component_registry_) {
        return;
    }
    
    Reality current_reality = Reality::A; 
    if (reality_manager_) {
        current_reality = reality_manager_->get_current_reality();
    }
    
    const auto* quantum_node_container = component_registry_->get_all_components<QuantumNode>();
    if (!quantum_node_container) {
        return;
    }
    
    const auto& entities = quantum_node_container->get_entities();
    const auto& quantum_nodes = quantum_node_container->get_components();
    
    for (size_t i = 0; i < entities.size(); ++i) {
        const EntityID entity = entities[i];
        const QuantumNode& quantum_node = quantum_nodes[i];
        
        const Transform* transform = component_registry_->get_component<Transform>(entity);
        if (!transform) {
        }
        
        render_quantum_node(renderer, entity, *transform, quantum_node, current_reality);
    }
}

void QuantumNodeRenderer::shutdown() {
    camera_controller_ = nullptr;
    reality_manager_ = nullptr;
    std::cout << "QuantumNodeRenderer shutdown" << std::endl;
}

void QuantumNodeRenderer::render_quantum_node(SDL_Renderer* renderer, EntityID entity,
                                             const Transform& transform, const QuantumNode& quantum_node,
                                             Reality current_reality) {
    int screen_x, screen_y;
    if (camera_controller_) {
        camera_controller_->world_to_screen(transform.x, transform.y, screen_x, screen_y);
    } else {
        screen_x = static_cast<int>(transform.x);
        screen_y = static_cast<int>(transform.y);
    }
    
    SDL_FColor node_color = get_quantum_node_color(quantum_node, current_reality);
    SDL_FColor glow_color = get_quantum_node_glow_color(quantum_node, current_reality);
    
    float pulse_intensity = calculate_pulse_intensity();
    
    render_glow_effect(renderer, static_cast<float>(screen_x), static_cast<float>(screen_y),
                      visual_config_.glow_radius, glow_color, pulse_intensity);
    
    float node_size = 24.0f * transform.scale_x; 
    SDL_FRect node_rect = {
        static_cast<float>(screen_x) - node_size * 0.5f,
        static_cast<float>(screen_y) - node_size * 0.5f,
        node_size,
        node_size
    };
    
    SDL_SetRenderDrawColorFloat(renderer, node_color.r, node_color.g, node_color.b, node_color.a);
    SDL_RenderFillRect(renderer, &node_rect);
    
    SDL_SetRenderDrawColorFloat(renderer, 1.0f, 1.0f, 1.0f, 0.8f);
    SDL_RenderRect(renderer, &node_rect);
    
    float core_size = node_size * 0.6f;
    SDL_FRect core_rect = {
        static_cast<float>(screen_x) - core_size * 0.5f,
        static_cast<float>(screen_y) - core_size * 0.5f,
        core_size,
        core_size
    };
    
    SDL_FColor pulsed_color = node_color;
    pulsed_color.a *= pulse_intensity;
    SDL_SetRenderDrawColorFloat(renderer, pulsed_color.r, pulsed_color.g, pulsed_color.b, pulsed_color.a);
    SDL_RenderFillRect(renderer, &core_rect);
    
    const InteractionPrompt* prompt = component_registry_->get_component<InteractionPrompt>(entity);
    if (prompt && prompt->is_visible) {
        render_interaction_prompt(renderer, static_cast<float>(screen_x), 
                                 static_cast<float>(screen_y), quantum_node, current_reality);
    }
}

void QuantumNodeRenderer::render_glow_effect(SDL_Renderer* renderer, float x, float y, 
                                            float radius, const SDL_FColor& color, float intensity) {
    int glow_steps = static_cast<int>(radius / 2.0f);
    
    for (int step = 0; step < glow_steps; ++step) {
        float step_ratio = static_cast<float>(step) / static_cast<float>(glow_steps);
        float alpha = color.a * intensity * (1.0f - step_ratio);
        float current_radius = radius * step_ratio;
        
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, alpha);
        
        int circle_segments = 16;
        for (int i = 0; i < circle_segments; ++i) {
            float angle = (static_cast<float>(i) / static_cast<float>(circle_segments)) * 2.0f * M_PI;
            float next_angle = (static_cast<float>(i + 1) / static_cast<float>(circle_segments)) * 2.0f * M_PI;
            
            float x1 = x + std::cos(angle) * current_radius;
            float y1 = y + std::sin(angle) * current_radius;
            float x2 = x + std::cos(next_angle) * current_radius;
            float y2 = y + std::sin(next_angle) * current_radius;
            
            SDL_RenderLine(renderer, static_cast<int>(x1), static_cast<int>(y1), 
                          static_cast<int>(x2), static_cast<int>(y2));
        }
    }
}

void QuantumNodeRenderer::render_interaction_prompt(SDL_Renderer* renderer, float x, float y,
                                                   const QuantumNode& quantum_node, Reality current_reality) {
    float prompt_x = x;
    float prompt_y = y + visual_config_.prompt_offset_y;
    
    std::string item_name = (current_reality == Reality::A) ? 
        quantum_node.reality_a_item : quantum_node.reality_b_item;
    
    float prompt_width = 80.0f;
    float prompt_height = 20.0f;
    
    SDL_FRect prompt_bg = {
        prompt_x - prompt_width * 0.5f,
        prompt_y - prompt_height * 0.5f,
        prompt_width,
        prompt_height
    };
    
    SDL_SetRenderDrawColorFloat(renderer, 
                               visual_config_.prompt_background_color.r,
                               visual_config_.prompt_background_color.g,
                               visual_config_.prompt_background_color.b,
                               visual_config_.prompt_background_color.a);
    SDL_RenderFillRect(renderer, &prompt_bg);
    
    SDL_SetRenderDrawColorFloat(renderer, 
                               visual_config_.prompt_text_color.r,
                               visual_config_.prompt_text_color.g,
                               visual_config_.prompt_text_color.b,
                               visual_config_.prompt_text_color.a);
    SDL_RenderRect(renderer, &prompt_bg);
    
    float e_size = 8.0f;
    SDL_RenderLine(renderer, prompt_x - e_size, prompt_y - e_size, prompt_x - e_size, prompt_y + e_size);
    SDL_RenderLine(renderer, prompt_x - e_size, prompt_y - e_size, prompt_x + e_size, prompt_y - e_size);
    SDL_RenderLine(renderer, prompt_x - e_size, prompt_y, prompt_x + e_size * 0.5f, prompt_y);
    SDL_RenderLine(renderer, prompt_x - e_size, prompt_y + e_size, prompt_x + e_size, prompt_y + e_size);
}

float QuantumNodeRenderer::calculate_pulse_intensity() const {
    float pulse = std::sin(animation_time_ * visual_config_.pulse_speed);
    float normalized_pulse = (pulse + 1.0f) * 0.5f; 
    
    return visual_config_.pulse_min + 
           (normalized_pulse * (visual_config_.pulse_max - visual_config_.pulse_min));
}

SDL_FColor QuantumNodeRenderer::get_quantum_node_color(const QuantumNode& quantum_node, Reality current_reality) const {
    if (quantum_node.is_activated) {
        return visual_config_.activated_color;
    }
    
    return (current_reality == Reality::A) ? 
        visual_config_.reality_a_color : visual_config_.reality_b_color;
}

SDL_FColor QuantumNodeRenderer::get_quantum_node_glow_color(const QuantumNode& quantum_node, Reality current_reality) const {
    if (quantum_node.is_activated) {
        return visual_config_.activated_glow_color;
    }
    
    return (current_reality == Reality::A) ? 
        visual_config_.reality_a_glow_color : visual_config_.reality_b_glow_color;
}