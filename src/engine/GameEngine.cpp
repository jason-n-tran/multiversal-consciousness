#include "GameEngine.h"
#include <iostream>
#include <chrono>

GameEngine::GameEngine() 
    : is_running_(false), is_initialized_(false) {
}

GameEngine::~GameEngine() {
    shutdown();
}

bool GameEngine::initialize_sdl() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool GameEngine::create_window(const EngineConfig& config) {
    Uint32 window_flags = 0;
    if (config.fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    window_ = WindowPtr(SDL_CreateWindow(
        config.window_title.c_str(),
        config.window_width,
        config.window_height,
        window_flags
    ));
    
    if (!window_) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    return true;
}

bool GameEngine::create_renderer(const EngineConfig& config) {
    renderer_ = RendererPtr(SDL_CreateRenderer(
        window_.get(),
        nullptr
    ));
    
    if (!renderer_) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (config.vsync) {
        SDL_SetRenderVSync(renderer_.get(), 1);
    }
    
    return true;
}

bool GameEngine::initialize_ecs() {
    entity_manager_ = std::make_unique<EntityManager>();
    component_registry_ = std::make_unique<ComponentRegistry>();
    system_manager_ = std::make_unique<SystemManager>(*entity_manager_, *component_registry_);
    
    system_manager_->initialize();
    
    return true;
}

bool GameEngine::initialize(const EngineConfig& config) {
    if (is_initialized_) {
        std::cerr << "Engine already initialized" << std::endl;
        return false;
    }
    
    // Initialize SDL3 subsystems
    if (!initialize_sdl()) {
        return false;
    }
    
    // Create window
    if (!create_window(config)) {
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    if (!create_renderer(config)) {
        SDL_Quit();
        return false;
    }

    if (!initialize_ecs()) {
        SDL_Quit();
        return false;
    }
    
    is_initialized_ = true;
    std::cout << "Game engine initialized successfully" << std::endl;
    return true;
}

void GameEngine::run() {
    if (!is_initialized_) {
        std::cerr << "Engine not initialized. Call initialize() first." << std::endl;
        return;
    }
    
    is_running_ = true;
    SDL_Event event;
    
    std::cout << "Starting main game loop..." << std::endl;

    auto last_time = std::chrono::high_resolution_clock::now();
    
    while (is_running_) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto delta_duration = current_time - last_time;
        float delta_time = std::chrono::duration<float>(delta_duration).count();
        last_time = current_time;
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                is_running_ = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    is_running_ = false;
                }
            }
        }
        system_manager_->update(delta_time);

        
        SDL_SetRenderDrawColor(renderer_.get(), 25, 25, 50, 255);
        SDL_RenderClear(renderer_.get());
        system_manager_->render(renderer_.get());
        
        SDL_RenderPresent(renderer_.get());
        
        SDL_Delay(16);
    }
    
    std::cout << "Main game loop ended" << std::endl;
}

void GameEngine::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    is_running_ = false;
    if (system_manager_) {
        system_manager_->shutdown();
    }
    
    system_manager_.reset();
    component_registry_.reset();
    entity_manager_.reset();
    renderer_.reset();
    window_.reset();
    
    SDL_Quit();
    
    is_initialized_ = false;
    std::cout << "Game engine shutdown complete" << std::endl;
}