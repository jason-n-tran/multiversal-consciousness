#include "AgentRenderer.h"
#include <iostream>
#include <cmath>

void AgentRenderer::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    std::cout << "AgentRenderer initialized" << std::endl;
}

void AgentRenderer::update(float delta_time) {
    animation_time_ += delta_time;
}

void AgentRenderer::render(SDL_Renderer* renderer) {
    if (!component_registry_) {
        return;
    }
    Reality current_reality = Reality::A;
    if (reality_manager_) {
        current_reality = reality_manager_->get_current_reality();
    }
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& entities = agent_container->get_entities();
    const auto& agents = agent_container->get_components();
    
    for (size_t i = 0; i < entities.size(); ++i) {
        const EntityID entity = entities[i];
        const Agent& agent = agents[i];
        
        const Transform* transform = component_registry_->get_component<Transform>(entity);
        if (!transform) {
            continue;
        }
        
        const Renderable* renderable = component_registry_->get_component<Renderable>(entity);

        if (renderable) {
            render_agent_with_animation(renderer, entity, *transform, agent, *renderable, current_reality);
        }
        
        render_agent_feedback(renderer, entity, *transform, agent, renderable);
    }
}

void AgentRenderer::shutdown() {
    camera_controller_ = nullptr;
    std::cout << "AgentRenderer shutdown" << std::endl;
}

void AgentRenderer::render_agent_feedback(SDL_Renderer* renderer, EntityID entity, 
                                         const Transform& transform, const Agent& agent, 
                                         const Renderable* renderable) {
    int screen_x, screen_y;
    if (camera_controller_) {
        camera_controller_->world_to_screen(transform.x, transform.y, screen_x, screen_y);
    } else {
        screen_x = static_cast<int>(transform.x);
        screen_y = static_cast<int>(transform.y);
    }
    
    float agent_width = 32.0f;
    float agent_height = 32.0f;
    if (renderable) {
        agent_width = static_cast<float>(renderable->source_rect.w) * transform.scale_x;
        agent_height = static_cast<float>(renderable->source_rect.h) * transform.scale_y;
    }
    
    if (agent.is_possessed) {
        float glow_intensity = calculate_glow_intensity();
        render_glow(renderer, static_cast<float>(screen_x), static_cast<float>(screen_y), 
                   agent_width, agent_height, 
                   visual_config_.possessed_glow_color, 
                   visual_config_.possessed_glow_radius, glow_intensity);
        
        render_outline(renderer, static_cast<float>(screen_x), static_cast<float>(screen_y), 
                      agent_width, agent_height, 
                      visual_config_.possessed_outline_color, 
                      visual_config_.possessed_outline_width);
    } else {
        render_outline(renderer, static_cast<float>(screen_x), static_cast<float>(screen_y), 
                      agent_width, agent_height, 
                      visual_config_.idle_outline_color, 
                      visual_config_.idle_outline_width);
    }
    
    render_agent_number(renderer, static_cast<float>(screen_x), 
                       static_cast<float>(screen_y) + visual_config_.number_offset_y, 
                       agent.agent_number);
}

void AgentRenderer::render_outline(SDL_Renderer* renderer, float x, float y, 
                                  float width, float height, 
                                  const SDL_FColor& color, float line_width) {
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    
    float half_width = width * 0.5f;
    float half_height = height * 0.5f;
    float left = x - half_width - line_width;
    float right = x + half_width + line_width;
    float top = y - half_height - line_width;
    float bottom = y + half_height + line_width;
    
    for (int i = 0; i < static_cast<int>(line_width); ++i) {
        float offset = static_cast<float>(i);
        
        SDL_RenderLine(renderer, left - offset, top - offset, right + offset, top - offset);
        SDL_RenderLine(renderer, left - offset, bottom + offset, right + offset, bottom + offset);
        SDL_RenderLine(renderer, left - offset, top - offset, left - offset, bottom + offset);
        SDL_RenderLine(renderer, right + offset, top - offset, right + offset, bottom + offset);
    }
}

void AgentRenderer::render_glow(SDL_Renderer* renderer, float x, float y, 
                               float width, float height, 
                               const SDL_FColor& color, float radius, float intensity) {
    int glow_steps = static_cast<int>(radius);
    
    for (int step = 0; step < glow_steps; ++step) {
        float step_ratio = static_cast<float>(step) / static_cast<float>(glow_steps);
        float alpha = color.a * intensity * (1.0f - step_ratio);
        
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, alpha);
        
        float glow_offset = radius * step_ratio;
        float half_width = width * 0.5f;
        float half_height = height * 0.5f;
        
        SDL_FRect glow_rect = {
            x - half_width - glow_offset,
            y - half_height - glow_offset,
            width + (glow_offset * 2.0f),
            height + (glow_offset * 2.0f)
        };
        
        SDL_RenderRect(renderer, &glow_rect);
    }
}

void AgentRenderer::render_agent_number(SDL_Renderer* renderer, float x, float y, uint8_t agent_number) {
    if (agent_number < 1 || agent_number > 9) {
        return;
    }
    
    SDL_SetRenderDrawColorFloat(renderer, 
                               visual_config_.number_background_color.r,
                               visual_config_.number_background_color.g,
                               visual_config_.number_background_color.b,
                               visual_config_.number_background_color.a);
    
    float circle_radius = 10.0f;
    SDL_FRect background_rect = {
        x - circle_radius,
        y - circle_radius,
        circle_radius * 2.0f,
        circle_radius * 2.0f
    };
    SDL_RenderFillRect(renderer, &background_rect);
    
    SDL_SetRenderDrawColorFloat(renderer, 
                               visual_config_.number_text_color.r,
                               visual_config_.number_text_color.g,
                               visual_config_.number_text_color.b,
                               visual_config_.number_text_color.a);
    
    float line_length = 6.0f;
    switch (agent_number) {
        case 1:
            SDL_RenderLine(renderer, x, y - line_length, x, y + line_length);
            break;
        case 2:
            SDL_RenderLine(renderer, x - line_length/2, y - line_length, x + line_length/2, y - line_length);
            SDL_RenderLine(renderer, x + line_length/2, y - line_length, x + line_length/2, y);
            SDL_RenderLine(renderer, x + line_length/2, y, x - line_length/2, y);
            SDL_RenderLine(renderer, x - line_length/2, y, x - line_length/2, y + line_length);
            SDL_RenderLine(renderer, x - line_length/2, y + line_length, x + line_length/2, y + line_length);
            break;
        default:
            SDL_RenderRect(renderer, &background_rect);
            break;
    }
}

float AgentRenderer::calculate_glow_intensity() const {
    float pulse = std::sin(animation_time_ * visual_config_.glow_pulse_speed);
    float normalized_pulse = (pulse + 1.0f) * 0.5f;
    return visual_config_.glow_pulse_min + 
           (normalized_pulse * (visual_config_.glow_pulse_max - visual_config_.glow_pulse_min));
}

void AgentRenderer::set_camera_controller(CameraController* camera_controller) {
    camera_controller_ = camera_controller;
}

void AgentRenderer::set_visual_config(const AgentVisualConfig& config) {
    visual_config_ = config;
}

void AgentRenderer::render_agent_with_animation(SDL_Renderer* renderer, EntityID entity,
                                               const Transform& transform, const Agent& agent,
                                               const Renderable& renderable, Reality current_reality) {
    int screen_x, screen_y;
    if (camera_controller_) {
        camera_controller_->world_to_screen(transform.x, transform.y, screen_x, screen_y);
    } else {
        screen_x = static_cast<int>(transform.x);
        screen_y = static_cast<int>(transform.y);
    }
    
    SDL_Rect animation_frame = get_animation_frame(agent, current_reality, animation_time_);
    
    float agent_width = static_cast<float>(animation_frame.w) * transform.scale_x;
    float agent_height = static_cast<float>(animation_frame.h) * transform.scale_y;
    
    SDL_FRect dest_rect = {
        static_cast<float>(screen_x) - agent_width * 0.5f,
        static_cast<float>(screen_y) - agent_height * 0.5f,
        agent_width,
        agent_height
    };
    
    SDL_FColor render_color = {renderable.color_r, renderable.color_g, renderable.color_b, renderable.color_a};
    
    if (agent.is_possessed) {
        render_color.r = std::min(1.0f, render_color.r * 1.1f);
        render_color.g = std::min(1.0f, render_color.g * 1.1f);
        render_color.b = std::min(1.0f, render_color.b * 1.1f);
    } else {
        render_color.r *= 0.9f;
        render_color.g *= 0.9f;
        render_color.b *= 0.9f;
    }
    
    SDL_SetRenderDrawColorFloat(renderer, render_color.r, render_color.g, render_color.b, render_color.a);
    SDL_RenderFillRect(renderer, &dest_rect);
    
    SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderRect(renderer, &dest_rect);
}

SDL_Rect AgentRenderer::get_animation_frame(const Agent& agent, Reality current_reality, float animation_time) const {
    int frame_width = 32;
    int frame_height = 32;
    
    int frame_x = 0;
    int frame_y = 0;
    
    if (agent.is_possessed) {
        int frame_count = 4; 
        float frame_duration = 0.2f; 
        int current_frame = static_cast<int>(animation_time / frame_duration) % frame_count;
        frame_x = current_frame * frame_width;
        
        frame_y = (current_reality == Reality::A) ? 0 : frame_height;
    } else {
        frame_x = 0;
        frame_y = (current_reality == Reality::A) ? frame_height * 2 : frame_height * 3;
    }
    
    return SDL_Rect{frame_x, frame_y, frame_width, frame_height};
}