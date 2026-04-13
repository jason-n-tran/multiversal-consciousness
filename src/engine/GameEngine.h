#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include "SDLDeleter.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "System.h"

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
    std::unique_ptr<EntityManager> entity_manager_;
    std::unique_ptr<ComponentRegistry> component_registry_;
    std::unique_ptr<SystemManager> system_manager_;
    
    bool initialize_sdl();
    bool create_window(const EngineConfig& config);
    bool create_renderer(const EngineConfig& config);
    bool initialize_ecs();
    
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

    EntityManager& get_entity_manager() { return *entity_manager_; }

    ComponentRegistry& get_component_registry() { return *component_registry_; }

    SystemManager& get_system_manager() { return *system_manager_; }
    
    template<typename T>
    T* register_system(std::unique_ptr<T> system) {
        return system_manager_->register_system(std::move(system));
    }
};