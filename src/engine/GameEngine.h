#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include "SDLDeleter.h"


struct EngineConfig {
    std::string window_title = "Multiversal Consciousness";
    int window_width = 1280;
    int window_height = 720;
    bool fullscreen = false;
    bool vsync = true;
};


class GameEngine {
private:
    WindowPtr window_;
    RendererPtr renderer_;
    bool is_running_;
    bool is_initialized_;
    
    bool initialize_sdl();
    bool create_window(const EngineConfig& config);
    bool create_renderer(const EngineConfig& config);
    
public:
    GameEngine();
    ~GameEngine();
    
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;
    
    GameEngine(GameEngine&&) noexcept = default;
    GameEngine& operator=(GameEngine&&) noexcept = default;
    
    bool initialize(const EngineConfig& config = {});
    
    void run();
    
    void shutdown();
    
    bool is_running() const { return is_running_; }

    bool is_initialized() const { return is_initialized_; }

    SDL_Window* get_window() const { return window_.get(); }

    SDL_Renderer* get_renderer() const { return renderer_.get(); }
};