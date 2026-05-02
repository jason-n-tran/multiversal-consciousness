#include "AgentRenderer.h"
#include <iostream>
#include <cmath>

void AgentRenderer::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    std::cout << "AgentRenderer initialized" << std::endl;
}

void AgentRenderer::initialize_sprites(SDL_Renderer* renderer) {
    sdl_renderer_ = renderer;

    load_agent_sprites(1, "assets/City_men_1");
    load_agent_sprites(2, "assets/City_men_2");
    load_agent_sprites(3, "assets/City_men_3");
    
    std::cout << "AgentRenderer sprites initialized" << std::endl;
}

bool AgentRenderer::load_agent_sprites(uint8_t agent_number, const std::string& base_path) {
    if (!sdl_renderer_) {
        std::cerr << "Cannot load sprites: SDL renderer not set" << std::endl;
        return false;
    }
    
    AgentSpriteData sprite_data;
    
    std::string idle_path = base_path + "/Idle.png";
    SDL_Surface* idle_surface = IMG_Load(idle_path.c_str());
    if (!idle_surface) {
        std::cerr << "Failed to load idle sprite: " << idle_path << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    sprite_data.idle.texture = SDL_CreateTextureFromSurface(sdl_renderer_, idle_surface);
    
    std::cout << "Idle sprite sheet: " << idle_surface->w << "x" << idle_surface->h << " pixels" << std::endl;
    
    sprite_data.idle.frame_width = 128;
    sprite_data.idle.frame_height = 128;
    sprite_data.idle.frame_count = idle_surface->w / 128;  // Calculate from actual width
    sprite_data.idle.frame_duration = 0.15f;
    sprite_data.sprite_width = sprite_data.idle.frame_width;
    sprite_data.sprite_height = sprite_data.idle.frame_height;
    
    std::cout << "Idle: " << sprite_data.idle.frame_count << " frames of " 
              << sprite_data.idle.frame_width << "x" << sprite_data.idle.frame_height << std::endl;
    
    SDL_DestroySurface(idle_surface);
    
    if (!sprite_data.idle.texture) {
        std::cerr << "Failed to create texture from idle sprite: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::string run_path = base_path + "/Run.png";
    SDL_Surface* run_surface = IMG_Load(run_path.c_str());
    if (!run_surface) {
        std::cerr << "Failed to load run sprite: " << run_path << " - " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(sprite_data.idle.texture);
        return false;
    }
    
    sprite_data.run.texture = SDL_CreateTextureFromSurface(sdl_renderer_, run_surface);
    
    std::cout << "Run sprite sheet: " << run_surface->w << "x" << run_surface->h << " pixels" << std::endl;
    
    sprite_data.run.frame_width = 128;
    sprite_data.run.frame_height = 128;
    sprite_data.run.frame_count = run_surface->w / 128;  // Calculate from actual width
    sprite_data.run.frame_duration = 0.08f;
    
    std::cout << "Run: " << sprite_data.run.frame_count << " frames of " 
              << sprite_data.run.frame_width << "x" << sprite_data.run.frame_height << std::endl;
    
    SDL_DestroySurface(run_surface);
    
    if (!sprite_data.run.texture) {
        std::cerr << "Failed to create texture from run sprite: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(sprite_data.idle.texture);
        return false;
    }
    
    agent_sprites_[agent_number] = sprite_data;
    std::cout << "Loaded sprites for agent " << static_cast<int>(agent_number) 
              << " (size: " << sprite_data.sprite_width << "x" << sprite_data.sprite_height << ")" << std::endl;
    
    return true;
}

void AgentRenderer::update(float delta_time) {
    animation_time_ += delta_time;
    
    if (!component_registry_) {
        return;
    }
    
    const auto* agent_container = component_registry_->get_all_components<Agent>();
    if (!agent_container) {
        return;
    }
    
    const auto& entities = agent_container->get_entities();
    for (EntityID entity : entities) {
        const PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
        AnimationState new_state = get_animation_state(entity, physics);
        
        update_entity_animation(entity, new_state, delta_time);
        
        if (physics && std::abs(physics->velocity_x) > 0.1f) {
            entity_facing_right_[entity] = physics->velocity_x > 0.0f;
        }
    }
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
        
        Renderable dummy_renderable;
        if (!renderable) {
            renderable = &dummy_renderable;
        }
        
        render_agent_with_animation(renderer, entity, *transform, agent, *renderable, current_reality);
        
        render_agent_feedback(renderer, entity, *transform, agent, renderable);
    }
}

void AgentRenderer::shutdown() {
    for (auto& [agent_num, sprite_data] : agent_sprites_) {
        if (sprite_data.idle.texture) {
            SDL_DestroyTexture(sprite_data.idle.texture);
        }
        if (sprite_data.run.texture) {
            SDL_DestroyTexture(sprite_data.run.texture);
        }
    }
    agent_sprites_.clear();
    entity_animation_states_.clear();
    entity_animation_times_.clear();
    entity_facing_right_.clear();
    
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
    
    float scale_factor = 32.0f / 128.0f;
    float agent_width = 128.0f * scale_factor * transform.scale_x;
    float agent_height = 128.0f * scale_factor * transform.scale_y;
    
    int visual_center_y = screen_y;
    
    if (agent.is_possessed) {
    } 
    
    float number_draw_y = static_cast<float>(visual_center_y) - agent_height * 0.5f + visual_config_.number_offset_y;
    
    render_agent_number(renderer, static_cast<float>(screen_x), number_draw_y, agent.agent_number);
                       
    if (agent.is_possessed) {
        float number_size = 24.0f; 
        render_outline(renderer, static_cast<float>(screen_x), number_draw_y, 
                      number_size, number_size, 
                      visual_config_.possessed_outline_color, 
                      visual_config_.possessed_outline_width);
    }
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
        case 3:
            // Simple "3" shape
            SDL_RenderLine(renderer, x - line_length/2, y - line_length, x + line_length/2, y - line_length);
            SDL_RenderLine(renderer, x + line_length/2, y - line_length, x + line_length/2, y);
            SDL_RenderLine(renderer, x - line_length/2, y, x + line_length/2, y);
            SDL_RenderLine(renderer, x + line_length/2, y, x + line_length/2, y + line_length);
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
    auto sprite_it = agent_sprites_.find(agent.agent_number);
    if (sprite_it == agent_sprites_.end()) {
        render_simple_agent(renderer, entity, transform, agent, renderable);
        return;
    }
    
    const AgentSpriteData& sprite_data = sprite_it->second;
    
    auto state_it = entity_animation_states_.find(entity);
    AnimationState current_state = (state_it != entity_animation_states_.end()) ? state_it->second : AnimationState::Idle;
    
    float anim_time = 0.0f;
    auto time_it = entity_animation_times_.find(entity);
    if (time_it != entity_animation_times_.end()) {
        anim_time = time_it->second;
    }
    
    const SpriteAnimation* current_anim = nullptr;
    switch (current_state) {
        case AnimationState::Idle:
            current_anim = &sprite_data.idle;
            break;
        case AnimationState::Jump:
        case AnimationState::Fall:
        case AnimationState::Run:
            current_anim = &sprite_data.run;
            break;
    }
    
    if (!current_anim || !current_anim->texture) {
        render_simple_agent(renderer, entity, transform, agent, renderable);
        return;
    }
    
    int current_frame = static_cast<int>(anim_time / current_anim->frame_duration) % current_anim->frame_count;
    
    SDL_FRect src_rect = {
        static_cast<float>(current_frame * current_anim->frame_width),
        0.0f,
        static_cast<float>(current_anim->frame_width),
        static_cast<float>(current_anim->frame_height)
    };
    
    int screen_x, screen_y;
    if (camera_controller_) {
        camera_controller_->world_to_screen(transform.x, transform.y, screen_x, screen_y);
    } else {
        screen_x = static_cast<int>(transform.x);
        screen_y = static_cast<int>(transform.y);
    }
    
    float scale_factor = 64.0f / 128.0f;
    float render_width = 128.0f * scale_factor * transform.scale_x;
    float render_height = 128.0f * scale_factor * transform.scale_y;
    
    float physics_bottom_offset = 16.0f * transform.scale_y;
    
    SDL_FRect dest_rect = {
        static_cast<float>(screen_x) - render_width * 0.5f,
        static_cast<float>(screen_y) + physics_bottom_offset - render_height,
        render_width,
        render_height
    };
    
    bool facing_right = true;
    auto facing_it = entity_facing_right_.find(entity);
    if (facing_it != entity_facing_right_.end()) {
        facing_right = facing_it->second;
    }
    
    SDL_FlipMode flip = facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    
    SDL_RenderTextureRotated(renderer, current_anim->texture, &src_rect, &dest_rect, 
                            0.0, nullptr, flip);
}

void AgentRenderer::render_simple_agent(SDL_Renderer* renderer, EntityID entity,
                                       const Transform& transform, const Agent& agent,
                                       const Renderable& renderable) {
    int screen_x, screen_y;
    if (camera_controller_) {
        camera_controller_->world_to_screen(transform.x, transform.y, screen_x, screen_y);
    } else {
        screen_x = static_cast<int>(transform.x);
        screen_y = static_cast<int>(transform.y);
    }
    
    float agent_width = 32.0f * transform.scale_x;
    float agent_height = 32.0f * transform.scale_y;
    
    SDL_FRect dest_rect = {
        static_cast<float>(screen_x) - agent_width * 0.5f,
        static_cast<float>(screen_y) - agent_height * 0.5f,
        agent_width,
        agent_height
    };
    
    SDL_SetRenderDrawColorFloat(renderer, renderable.color_r, renderable.color_g, 
                               renderable.color_b, renderable.color_a);
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

AnimationState AgentRenderer::get_animation_state(EntityID entity, const PhysicsComponent* physics) const {
    if (!physics) {
        return AnimationState::Idle;
    }
    
    bool is_moving = std::abs(physics->velocity_x) > 0.5f;
    
    bool is_grounded = physics->is_grounded;
    
    if (!is_grounded) {
        if (physics->velocity_y < -0.5f) {
            return AnimationState::Jump;
        } else {
            return AnimationState::Fall;
        }
    } else if (is_moving) {
        return AnimationState::Run;
    } else {
        return AnimationState::Idle;
    }
}

void AgentRenderer::update_entity_animation(EntityID entity, AnimationState new_state, float delta_time) {
    auto state_it = entity_animation_states_.find(entity);
    AnimationState current_state = (state_it != entity_animation_states_.end()) ? 
                                   state_it->second : AnimationState::Idle;
    
    if (current_state != new_state) {
        entity_animation_times_[entity] = 0.0f;
        entity_animation_states_[entity] = new_state;
    } else {
        entity_animation_times_[entity] += delta_time;
    }
}