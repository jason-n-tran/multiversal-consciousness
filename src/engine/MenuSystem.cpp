#include "MenuSystem.h"
#include "GameEngine.h"
#include <iostream>

MenuSystem::MenuSystem(GameEngine* engine) : engine_(engine) {
}

MenuSystem::~MenuSystem() {
    if (font_) {
        TTF_CloseFont(font_);
    }
    if (font_small_) {
        TTF_CloseFont(font_small_);
    }
}

void MenuSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);

    if (!TTF_WasInit()) {
        TTF_Init();
    }

    const char* font_paths[] = {
        "assets/arial.ttf",
        "assets/ARIAL.TTF",
        "arial.ttf",
        "ARIAL.TTF",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for (const char* path : font_paths) {
        font_ = TTF_OpenFont(path, 32);
        if (font_) {
            font_small_ = TTF_OpenFont(path, 20);
            std::cout << "MenuSystem: Loaded font from " << path << std::endl;
            break;
        }
    }

    if (!font_) {
        std::cerr << "MenuSystem: Failed to load font! Tried: ";
        for(auto p : font_paths) std::cerr << p << ", ";
        std::cerr << std::endl;
    }

    rebuild_buttons();
}

void MenuSystem::rebuild_buttons() {
    buttons_.clear();
    
    float center_x = 1280.0f / 2.0f;
    float start_y = 250.0f;
    float spacing = 60.0f;
    float btn_w = 300.0f;
    float btn_h = 50.0f;

    if (menu_mode_ == MenuMode::Start) {
        buttons_.push_back({
            "Start Game",
            {center_x - btn_w/2, start_y, btn_w, btn_h},
            [this]() { 
                if (start_callback_) {
                    start_callback_();
                    engine_->set_paused(false);
                }
            }
        });
        
        buttons_.push_back({
            "Controls & Guide",
            {center_x - btn_w/2, start_y + spacing, btn_w, btn_h},
            [this]() { show_controls_ = true; }
        });

        buttons_.push_back({
            "Quit",
            {center_x - btn_w/2, start_y + spacing * 2, btn_w, btn_h},
            [this]() { 
                 SDL_Event quit_event;
                 quit_event.type = SDL_EVENT_QUIT;
                 SDL_PushEvent(&quit_event);
            }
        });
    } else if (menu_mode_ == MenuMode::Pause) {
        buttons_.push_back({
            "Resume",
            {center_x - btn_w/2, start_y, btn_w, btn_h},
            [this]() { engine_->set_paused(false); }
        });

        buttons_.push_back({
            "Reset Level",
            {center_x - btn_w/2, start_y + spacing, btn_w, btn_h},
            [this]() { 
                if (reset_callback_) {
                    reset_callback_();
                    engine_->set_paused(false);
                }
            }
        });

        buttons_.push_back({
            "Controls & Guide",
            {center_x - btn_w/2, start_y + spacing * 2, btn_w, btn_h},
            [this]() { show_controls_ = true; }
        });
        
        buttons_.push_back({
            "Quit",
            {center_x - btn_w/2, start_y + spacing * 3, btn_w, btn_h},
            [this]() { 
                 SDL_Event quit_event;
                 quit_event.type = SDL_EVENT_QUIT;
                 SDL_PushEvent(&quit_event);
            }
        });
    } else if (menu_mode_ == MenuMode::Congratulations) {
        buttons_.push_back({
            "Back to Main Menu",
            {center_x - btn_w/2, start_y + spacing * 2, btn_w, btn_h},
            [this]() { 
                set_menu_mode(MenuMode::Start);
            }
        });
        
        buttons_.push_back({
            "Quit",
            {center_x - btn_w/2, start_y + spacing * 3, btn_w, btn_h},
            [this]() { 
                 SDL_Event quit_event;
                 quit_event.type = SDL_EVENT_QUIT;
                 SDL_PushEvent(&quit_event);
            }
        });
    }
}

void MenuSystem::update(float delta_time) {
    if (!engine_->is_paused()) {
        show_controls_ = false; 
        return;
    }

    handle_input();
}

void MenuSystem::handle_input() {
    float mouse_x, mouse_y;
    SDL_MouseButtonFlags buttons_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    bool clicked = (buttons_state & SDL_BUTTON_LMASK);
    
    static bool was_clicked = false;
    bool just_clicked = clicked && !was_clicked;
    was_clicked = clicked;

    if (show_controls_) {
        if (just_clicked) {
            show_controls_ = false;
        }
        return;
    }

    for (auto& btn : buttons_) {
        bool hovered = (mouse_x >= btn.rect.x && mouse_x <= btn.rect.x + btn.rect.w &&
                        mouse_y >= btn.rect.y && mouse_y <= btn.rect.y + btn.rect.h);
        btn.is_hovered = hovered;
        
        if (hovered && just_clicked) {
            btn.action();
        }
    }
}

void MenuSystem::render(SDL_Renderer* renderer) {
    if (!renderer) return;
    if (!engine_->is_paused()) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); 
    SDL_FRect screen_rect = {0.0f, 0.0f, 1280.0f, 720.0f};
    SDL_RenderFillRect(renderer, &screen_rect);

    if (show_controls_) {
        render_controls(renderer);
    } else {
        render_main_menu(renderer);
    }
}

void MenuSystem::render_main_menu(SDL_Renderer* renderer) {
    if (!font_) return;
    
    SDL_Color white = {255, 255, 255, 255};
    std::string title_text = "GAME PAUSED";
    if (menu_mode_ == MenuMode::Start) title_text = "Multiversal Consciousness";
    else if (menu_mode_ == MenuMode::Congratulations) title_text = "CONGRATULATIONS!";

    SDL_Surface* title_surf = TTF_RenderText_Blended(font_, title_text.c_str(), 0, white);
    if (title_surf) {
        float title_w = (float)title_surf->w;
        float title_h = (float)title_surf->h;
        SDL_Texture* title_tex = SDL_CreateTextureFromSurface(renderer, title_surf);
        SDL_FRect title_rect = { (1280.0f - title_w) / 2.0f, 100.0f, title_w, title_h };
        SDL_RenderTexture(renderer, title_tex, nullptr, &title_rect);
        SDL_DestroyTexture(title_tex);
        SDL_DestroySurface(title_surf);
    }

    if (menu_mode_ == MenuMode::Congratulations) {
         render_text_centered(renderer, "You have solved all puzzles and mastered Multiversal Consciousness!", 1280.0f/2.0f, 200.0f, font_small_, {0.8f, 1.0f, 0.8f, 1.0f});
    }

    for (auto& btn : buttons_) {
        render_button(renderer, btn);
    }
}

void MenuSystem::render_button(SDL_Renderer* renderer, const MenuButton& button) {
    if (button.is_hovered) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
    }
    SDL_RenderFillRect(renderer, &button.rect);
    
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderRect(renderer, &button.rect);

    if (font_small_) {
         SDL_Color white = {255, 255, 255, 255};
         SDL_Surface* text_surf = TTF_RenderText_Blended(font_small_, button.text.c_str(), 0, white);
         if (text_surf) {
             float text_w = (float)text_surf->w;
             float text_h = (float)text_surf->h;
             SDL_Texture* text_tex = SDL_CreateTextureFromSurface(renderer, text_surf);
             
             SDL_FRect text_rect = {
                 button.rect.x + (button.rect.w - text_w) / 2.0f,
                 button.rect.y + (button.rect.h - text_h) / 2.0f,
                 text_w,
                 text_h
             };
             
             SDL_RenderTexture(renderer, text_tex, nullptr, &text_rect);
             SDL_DestroyTexture(text_tex);
             SDL_DestroySurface(text_surf);
         }
    }
}

void MenuSystem::render_controls(SDL_Renderer* renderer) {
    if (!font_small_) return;

    float cx = 1280.0f / 2.0f;
    float cy = 100.0f;
    float lh = 30.0f;
    
    render_text_centered(renderer, "CONTROLS & LEGEND", cx, 50.0f, font_, {1.0f, 1.0f, 0.0f, 1.0f});

    std::vector<std::string> lines = {
        "A / D : Move Left / Right",
        "W / Space : Jump (Double Jump available)",
        "1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 : Switch Agents",
        "R / TAB : Switch Reality (A <-> B)",
        "E : Interact (Unlock Doors, Chop Trees, Get Abilities)",
        "ESC : Pause / Menu",
        "",
        "--- VISUAL GUIDE ---",
        "Gray Blocks : Regular Walls",
        "Cyan Blocks : Phaseable Walls (Requires PhaseShift)",
        "Brown Blocks : Trees (Requires Axe)",
        "Gold Blocks : Doors (Requires Keycard)",
        "Blue Blocks : Water (Requires WaterWalk)",
        "Red Blocks : Switches",
        "Purple Squares : Quantum Nodes (Grant Abilities)",
        "Green Area : Success Zone",
        "",
        "(Click anywhere to return)"
    };

    for (size_t i = 0; i < lines.size(); ++i) {
        SDL_FColor color = {0.9f, 0.9f, 0.9f, 1.0f};
        if (lines[i].find("GUIDE") != std::string::npos) color = {1.0f, 1.0f, 0.0f, 1.0f};
        render_text_centered(renderer, lines[i], cx, cy + i * lh, font_small_, color);
    }
}

void MenuSystem::render_text_centered(SDL_Renderer* renderer, const std::string& text, float x, float y, TTF_Font* font, SDL_FColor color) {
    if (text.empty()) return;
    
    SDL_Color sdl_color = {
        static_cast<Uint8>(color.r * 255),
        static_cast<Uint8>(color.g * 255),
        static_cast<Uint8>(color.b * 255),
        static_cast<Uint8>(color.a * 255)
    };
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, sdl_color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_FRect dest = {x - surface->w / 2.0f, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        SDL_RenderTexture(renderer, texture, nullptr, &dest);
        SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
}
