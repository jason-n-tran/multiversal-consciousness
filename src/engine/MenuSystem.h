#pragma once

#include "System.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <functional>
#include <string>
#include <vector>

class GameEngine;

enum class MenuMode {
    Start,
    Pause,
    Congratulations
};

struct MenuButton {
    std::string text;
    SDL_FRect rect;
    std::function<void()> action;
    bool is_hovered = false;
};

class MenuSystem : public IRenderSystem {
public:
    MenuSystem(GameEngine* engine);
    ~MenuSystem() override;

    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    
    bool run_while_paused() const override { return true; }

    void set_reset_callback(std::function<void()> callback) {
        reset_callback_ = callback;
    }

    void set_start_callback(std::function<void()> callback) {
        start_callback_ = callback;
    }

    void set_menu_mode(MenuMode mode) {
        menu_mode_ = mode;
        rebuild_buttons();
    }

private:
    GameEngine* engine_;
    TTF_Font* font_ = nullptr;
    TTF_Font* font_small_ = nullptr;
    
    std::vector<MenuButton> buttons_;
    std::function<void()> reset_callback_;
    std::function<void()> start_callback_;
    
    MenuMode menu_mode_ = MenuMode::Pause;
    bool show_controls_ = false;
    
    void rebuild_buttons();
    
    void handle_input();
    void render_main_menu(SDL_Renderer* renderer);
    void render_controls(SDL_Renderer* renderer);
    void render_button(SDL_Renderer* renderer, const MenuButton& button);
    
    void render_text_centered(SDL_Renderer* renderer, const std::string& text, float x, float y, TTF_Font* font, SDL_FColor color);
};
