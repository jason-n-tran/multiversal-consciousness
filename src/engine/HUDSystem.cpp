#include "HUDSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>

void HUDSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);
    
    if (!initialize_fonts()) {
        std::cerr << "Failed to initialize fonts for HUD system" << std::endl;
        return;
    }
    
    current_agent_number_ = 0;
    current_abilities_.clear();
    current_reality_ = Reality::A;
    
    agent_number_alpha_ = 1.0f;
    ability_alpha_ = 1.0f;
    update_animation_timer_ = 0.0f;
    needs_update_ = true;
    
    std::cout << "HUD System initialized with SDL_ttf" << std::endl;
}

void HUDSystem::update(float delta_time) {
    update_animations(delta_time);
    
    if (verification_message_timer_ > 0.0f) {
        verification_message_timer_ -= delta_time;
        if (verification_message_timer_ <= 0.0f) {
            verification_message_.clear();
        }
    }
    
    if (needs_update_ || has_state_changed()) {
        update_display_state();
        needs_update_ = false;
    }
}

void HUDSystem::render(SDL_Renderer* renderer) {
    if (!renderer || !ttf_initialized_) {
        return;
    }
    
    render_agent_number(renderer);
    
    render_abilities(renderer);
    
    render_verification_message(renderer);
}

void HUDSystem::shutdown() {
    cleanup_fonts();
    possession_system_ = nullptr;
    loadout_system_ = nullptr;
    reality_manager_ = nullptr;
}

bool HUDSystem::initialize_fonts() {
    if (!TTF_WasInit() && !TTF_Init()) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    ttf_initialized_ = true;
    
    const char* font_paths[] = {
        "assets/arial.ttf",
        "assets/ARIAL.TTF",
        "arial.ttf",
        "ARIAL.TTF",
        // Windows
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/arial.ttf",
        // macOS
        "/System/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "font.ttf"
    };
    
    const char* font_path = nullptr;
    for (const char* path : font_paths) {
        font_large_ = TTF_OpenFont(path, 24);
        if (font_large_) {
            font_path = path;
            std::cout << "HUDSystem: Loaded font from " << path << std::endl;
            break;
        }
    }
    
    if (!font_large_) {
        std::cerr << "Failed to load any font. Please ensure a TrueType font is available." << std::endl;
        std::cerr << "You can place arial.ttf or font.ttf in the project directory." << std::endl;
        cleanup_fonts();
        return false;
    }
    
    font_medium_ = TTF_OpenFont(font_path, 18);
    font_small_ = TTF_OpenFont(font_path, 14);
    
    if (!font_medium_ || !font_small_) {
        std::cerr << "Failed to load all font sizes" << std::endl;
        cleanup_fonts();
        return false;
    }
    
    std::cout << "Fonts loaded successfully from: " << font_path << std::endl;
    return true;
}

void HUDSystem::cleanup_fonts() {
    if (font_large_) {
        TTF_CloseFont(font_large_);
        font_large_ = nullptr;
    }
    if (font_medium_) {
        TTF_CloseFont(font_medium_);
        font_medium_ = nullptr;
    }
    if (font_small_) {
        TTF_CloseFont(font_small_);
        font_small_ = nullptr;
    }
    
    if (ttf_initialized_) {
        TTF_Quit();
        ttf_initialized_ = false;
    }
}

float HUDSystem::render_text(SDL_Renderer* renderer, const std::string& text, TTF_Font* font,
                            float x, float y, const SDL_FColor& color) {
    if (!font || text.empty()) {
        return 0.0f;
    }
    
    SDL_Color sdl_color = {
        static_cast<Uint8>(color.r * 255),
        static_cast<Uint8>(color.g * 255),
        static_cast<Uint8>(color.b * 255),
        static_cast<Uint8>(color.a * 255)
    };
    
    SDL_Surface* text_surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), text.length(), sdl_color, 0);
    if (!text_surface) {
        std::cerr << "Failed to create text surface: " << SDL_GetError() << std::endl;
        return 0.0f;
    }
    
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(text_surface);
        return 0.0f;
    }
    
    int text_width = text_surface->w;
    int text_height = text_surface->h;
    
    SDL_DestroySurface(text_surface);
    
    SDL_FRect dest_rect = {x, y, static_cast<float>(text_width), static_cast<float>(text_height)};
    SDL_RenderTexture(renderer, text_texture, nullptr, &dest_rect);
    
    SDL_DestroyTexture(text_texture);
    
    return static_cast<float>(text_height);
}

float HUDSystem::render_text_with_background(SDL_Renderer* renderer, const std::string& text, TTF_Font* font,
                                            float x, float y, const SDL_FColor& text_color, const SDL_FColor& bg_color) {
    if (!font || text.empty()) {
        return 0.0f;
    }
    
    SDL_Color sdl_color = {
        static_cast<Uint8>(text_color.r * 255),
        static_cast<Uint8>(text_color.g * 255),
        static_cast<Uint8>(text_color.b * 255),
        static_cast<Uint8>(text_color.a * 255)
    };

    SDL_Surface* text_surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), text.length(), sdl_color, 800);
    if (!text_surface) {
        std::cerr << "Failed to create text surface: " << SDL_GetError() << std::endl;
        return 0.0f;
    }
    
    int text_width = text_surface->w;
    int text_height = text_surface->h;

    float panel_x = x - visual_config_.panel_padding;
    float panel_y = y - visual_config_.panel_padding;
    float panel_width = static_cast<float>(text_width) + (visual_config_.panel_padding * 2);
    float panel_height = static_cast<float>(text_height) + (visual_config_.panel_padding * 2);
    
    render_panel(renderer, panel_x, panel_y, panel_width, panel_height, bg_color);
    
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(text_surface);
        return 0.0f;
    }

    SDL_DestroySurface(text_surface);
    
    SDL_FRect dest_rect = {x, y, static_cast<float>(text_width), static_cast<float>(text_height)};
    SDL_RenderTexture(renderer, text_texture, nullptr, &dest_rect);
    
    SDL_DestroyTexture(text_texture);

    return static_cast<float>(text_height);
}

void HUDSystem::set_possession_system(PossessionSystem* possession_system) {
    possession_system_ = possession_system;
    needs_update_ = true;
}

void HUDSystem::set_loadout_system(QuantumLoadoutSystem* loadout_system) {
    loadout_system_ = loadout_system;
    needs_update_ = true;
}

void HUDSystem::set_reality_manager(RealityManager* reality_manager) {
    reality_manager_ = reality_manager;
    needs_update_ = true;
}

void HUDSystem::set_visual_config(const HUDVisualConfig& config) {
    visual_config_ = config;
    needs_update_ = true;
}

void HUDSystem::update_display_state() {
    uint8_t new_agent_number = 0;
    if (possession_system_) {
        auto possessed_entity = possession_system_->get_possessed_entity();
        if (possessed_entity.has_value()) {
            new_agent_number = possession_system_->get_agent_number(possessed_entity.value());
        }
    }
    
    std::vector<std::string> new_abilities;
    Reality new_reality = Reality::A;
    
    if (reality_manager_) {
        new_reality = reality_manager_->get_current_reality();
    }
    
    if (loadout_system_ && possession_system_) {
        auto possessed_entity = possession_system_->get_possessed_entity();
        if (possessed_entity.has_value()) {
            AbilityType current_ability = loadout_system_->get_current_ability(possessed_entity.value());
            if (current_ability != AbilityType::None) {
                new_abilities.push_back(get_ability_display_name(current_ability));
            }
            
            Reality other_reality = (new_reality == Reality::A) ? Reality::B : Reality::A;
            AbilityType other_ability = loadout_system_->get_ability_for_reality(possessed_entity.value(), other_reality);
            if (other_ability != AbilityType::None) {
                std::string other_ability_name = get_ability_display_name(other_ability);
                other_ability_name += " (Reality " + std::string(other_reality == Reality::A ? "A" : "B") + ")";
                new_abilities.push_back(other_ability_name);
            }
        }
    }
    
    bool changed = (new_agent_number != current_agent_number_) ||
                   (new_abilities != current_abilities_) ||
                   (new_reality != current_reality_);
    
    if (changed) {
        update_animation_timer_ = visual_config_.update_animation_time;
        
        current_agent_number_ = new_agent_number;
        current_abilities_ = std::move(new_abilities);
        current_reality_ = new_reality;
        
        std::cout << "HUD updated: Agent " << static_cast<int>(current_agent_number_) 
                  << ", Reality " << (current_reality_ == Reality::A ? "A" : "B")
                  << ", Abilities: " << current_abilities_.size() << std::endl;
    }
}

void HUDSystem::render_agent_number(SDL_Renderer* renderer) {
    if (current_agent_number_ == 0) {
        return;
    }
    
    std::string agent_text = "Agent " + std::to_string(current_agent_number_);
    
    SDL_FColor text_color = visual_config_.agent_number_color;
    SDL_FColor bg_color = visual_config_.agent_number_bg_color;
    text_color.a *= agent_number_alpha_;
    bg_color.a *= agent_number_alpha_;
    
    render_text_with_background(renderer, agent_text, font_large_,
                               visual_config_.agent_number_x,
                               visual_config_.agent_number_y,
                               text_color, bg_color);
}

void HUDSystem::render_abilities(SDL_Renderer* renderer) {
    if (current_abilities_.empty()) {
        return;
    }
    
    float y_offset = visual_config_.ability_y;
    
    for (size_t i = 0; i < current_abilities_.size(); ++i) {
        const std::string& ability_text = current_abilities_[i];
        
        SDL_FColor text_color = visual_config_.ability_text_color;
        SDL_FColor bg_color = visual_config_.ability_bg_color;
        text_color.a *= ability_alpha_;
        bg_color.a *= ability_alpha_;
        
        if (ability_text.find("Reality") != std::string::npos) {
            text_color.a *= 0.6f;
        }
        
        float text_height = render_text_with_background(renderer, ability_text, font_medium_,
                                                       visual_config_.ability_x,
                                                       y_offset,
                                                       text_color, bg_color);
        
        y_offset += text_height + visual_config_.ability_spacing;
    }
}

void HUDSystem::render_panel(SDL_Renderer* renderer, float x, float y, 
                            float width, float height, const SDL_FColor& color) {
    SDL_FRect rect = {x, y, width, height};
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

std::string HUDSystem::get_ability_display_name(AbilityType ability) const {
    switch (ability) {
        case AbilityType::None:
            return "None";
        case AbilityType::Axe:
            return "Axe";
        case AbilityType::Keycard:
            return "Keycard";
        case AbilityType::DoubleJump:
            return "Double Jump";
        case AbilityType::Dash:
            return "Dash";
        case AbilityType::WaterWalk:
            return "Water Walk";
        case AbilityType::PhaseShift:
            return "Phase Shift";
        default:
            return "Unknown";
    }
}

bool HUDSystem::has_state_changed() const {
    uint8_t current_game_agent = 0;
    if (possession_system_) {
        auto possessed_entity = possession_system_->get_possessed_entity();
        if (possessed_entity.has_value()) {
            current_game_agent = possession_system_->get_agent_number(possessed_entity.value());
        }
    }
    
    Reality current_game_reality = Reality::A;
    if (reality_manager_) {
        current_game_reality = reality_manager_->get_current_reality();
    }
    
    return (current_game_agent != current_agent_number_) ||
           (current_game_reality != current_reality_);
}

void HUDSystem::update_animations(float delta_time) {
    if (update_animation_timer_ > 0.0f) {
        update_animation_timer_ -= delta_time;
        
        float progress = 1.0f - (update_animation_timer_ / visual_config_.update_animation_time);
        progress = std::clamp(progress, 0.0f, 1.0f);
        
        float eased_progress = progress * progress * (3.0f - 2.0f * progress);
        agent_number_alpha_ = 0.3f + (0.7f * eased_progress);
        ability_alpha_ = 0.3f + (0.7f * eased_progress);
    } else {
        agent_number_alpha_ = 1.0f;
        ability_alpha_ = 1.0f;
    }
}

void HUDSystem::on_possession_changed(uint8_t new_agent_number) {
    if (new_agent_number != current_agent_number_) {
        current_agent_number_ = new_agent_number;
        update_animation_timer_ = visual_config_.update_animation_time;
        needs_update_ = true;
    }
}

void HUDSystem::on_reality_changed(Reality new_reality) {
    if (new_reality != current_reality_) {
        current_reality_ = new_reality;
        update_animation_timer_ = visual_config_.update_animation_time;
        needs_update_ = true;
    }
}

void HUDSystem::on_abilities_changed(EntityID agent_entity) {
    if (possession_system_) {
        auto possessed_entity = possession_system_->get_possessed_entity();
        if (possessed_entity.has_value() && possessed_entity.value() == agent_entity) {
            update_animation_timer_ = visual_config_.update_animation_time;
            needs_update_ = true;
        }
    }
}

void HUDSystem::set_verification_message(const std::string& message, float duration) {
    verification_message_ = message;
    verification_message_timer_ = duration;
}

void HUDSystem::render_verification_message(SDL_Renderer* renderer) {
    if (verification_message_.empty() || verification_message_timer_ <= 0.0f) {
        return;
    }
    
    float alpha = 1.0f;
    if (verification_message_timer_ < 0.5f) {
        alpha = verification_message_timer_ / 0.5f;
    }
    
    float message_x = 400.0f; 
    float message_y = 600.0f; 
    
    SDL_FColor text_color = {1.0f, 1.0f, 0.0f, alpha}; 
    SDL_FColor bg_color = {0.0f, 0.0f, 0.0f, alpha * 0.8f}; 
    
    render_text_with_background(renderer, verification_message_, font_medium_,
                               message_x, message_y,
                               text_color, bg_color);
}