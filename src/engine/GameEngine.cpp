#include "GameEngine.h"
#include <iostream>
#include <chrono>
#include <thread>

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
    input_manager_ = std::make_unique<InputManager>();
    
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
    constexpr float TARGET_FPS = 60.0f;
    constexpr float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;
    constexpr std::chrono::microseconds TARGET_FRAME_DURATION(static_cast<long>(TARGET_FRAME_TIME * 1000000));
    
    float accumulated_time = 0.0f;
    int frame_count = 0;
    auto fps_timer = std::chrono::high_resolution_clock::now();
    
    while (is_running_) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        auto current_time = std::chrono::high_resolution_clock::now();
        auto delta_duration = current_time - last_time;
        float delta_time = std::chrono::duration<float>(delta_duration).count();
        constexpr float MAX_DELTA_TIME = 1.0f / 30.0f; 
        delta_time = std::min(delta_time, MAX_DELTA_TIME);
        last_time = current_time;
        // Handle events
        while (SDL_PollEvent(&event)) {
            bool handled_by_input = input_manager_->process_event(event);
            
            if (!handled_by_input) {
                if (event.type == SDL_EVENT_QUIT) {
                    is_running_ = false;
                }
            }
            
            if (input_manager_->is_action_just_pressed(InputAction::PAUSE)) {
                is_running_ = false;
            }
        }
        
        input_manager_->update(delta_time);
        system_manager_->update(delta_time);

        
        SDL_SetRenderDrawColor(renderer_.get(), 25, 25, 50, 255);
        SDL_RenderClear(renderer_.get());
        system_manager_->render(renderer_.get());
        
        SDL_RenderPresent(renderer_.get());
        
        auto frame_end = std::chrono::high_resolution_clock::now();
        auto frame_duration = frame_end - frame_start;
        
        if (frame_duration < TARGET_FRAME_DURATION) {
            auto sleep_duration = TARGET_FRAME_DURATION - frame_duration;
            std::this_thread::sleep_for(sleep_duration);
        }
        
        accumulated_time += delta_time;
        frame_count++;
        
        auto fps_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - fps_timer).count();
        
        if (fps_elapsed >= 1000) { 
            float avg_fps = frame_count / accumulated_time;
            
            if (frame_count % 60 == 0) { 
                std::string title = "Quantum Bifurcation - Complete Engine Integration (FPS: " + 
                                  std::to_string(static_cast<int>(avg_fps)) + ")";
                SDL_SetWindowTitle(window_.get(), title.c_str());
            }
            
            accumulated_time = 0.0f;
            frame_count = 0;
            fps_timer = current_time;
        }
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
    input_manager_.reset();
    renderer_.reset();
    window_.reset();
    
    SDL_Quit();
    
    is_initialized_ = false;
    std::cout << "Game engine shutdown complete" << std::endl;
}