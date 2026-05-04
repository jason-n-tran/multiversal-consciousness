#include <catch2/catch_all.hpp>
#include "engine/GameEngine.h"
#include "engine/SDLDeleter.h"
#include <memory>

TEST_CASE("SDL3 initialization and cleanup", "[engine][foundation]") {
    SECTION("Engine can be created and destroyed") {
        // Test basic construction/destruction
        auto engine = std::make_unique<GameEngine>();
        REQUIRE_FALSE(engine->is_initialized());
        REQUIRE_FALSE(engine->is_running());
    }
    
    SECTION("Engine initializes SDL3 subsystems successfully") {
        GameEngine engine;
        EngineConfig config;
        config.window_title = "Test Window";
        config.window_width = 800;
        config.window_height = 600;
        
        REQUIRE(engine.initialize(config));
        REQUIRE(engine.is_initialized());
        REQUIRE(engine.get_window() != nullptr);
        REQUIRE(engine.get_renderer() != nullptr);
        
        // Explicit shutdown for testing
        engine.shutdown();
        REQUIRE_FALSE(engine.is_initialized());
    }
    
    SECTION("Engine prevents double initialization") {
        GameEngine engine;
        EngineConfig config;
        
        REQUIRE(engine.initialize(config));
        REQUIRE_FALSE(engine.initialize(config)); // Second init should fail
    }
}

TEST_CASE("SDL custom deleters work correctly", "[engine][memory]") {
    SECTION("SDLDeleter can handle null pointers safely") {
        SDLDeleter deleter;
        
        // These should not crash
        deleter(static_cast<SDL_Window*>(nullptr));
        deleter(static_cast<SDL_Renderer*>(nullptr));
        deleter(static_cast<SDL_Texture*>(nullptr));
        deleter(static_cast<SDL_Surface*>(nullptr));
        
        SUCCEED("No crashes with null pointers");
    }
    
    SECTION("Smart pointers with SDL deleters can be created") {
        // Test that the type aliases work
        WindowPtr window_ptr;
        RendererPtr renderer_ptr;
        TexturePtr texture_ptr;
        SurfacePtr surface_ptr;
        
        REQUIRE(window_ptr == nullptr);
        REQUIRE(renderer_ptr == nullptr);
        REQUIRE(texture_ptr == nullptr);
        REQUIRE(surface_ptr == nullptr);
    }
}

TEST_CASE("Engine configuration handling", "[engine][config]") {
    SECTION("Default configuration works") {
        GameEngine engine;
        EngineConfig default_config;
        
        REQUIRE(engine.initialize(default_config));
        REQUIRE(engine.is_initialized());
    }
    
    SECTION("Custom configuration is applied") {
        GameEngine engine;
        EngineConfig config;
        config.window_title = "Custom Test Window";
        config.window_width = 1024;
        config.window_height = 768;
        config.fullscreen = false;
        config.vsync = false;
        
        REQUIRE(engine.initialize(config));
        REQUIRE(engine.is_initialized());
        
        // Verify window was created (we can't easily test exact dimensions without platform-specific code)
        REQUIRE(engine.get_window() != nullptr);
        REQUIRE(engine.get_renderer() != nullptr);
    }
}

TEST_CASE("RAII resource management", "[engine][memory][raii]") {
    SECTION("Resources are cleaned up automatically") {
        SDL_Window* window_ptr = nullptr;
        SDL_Renderer* renderer_ptr = nullptr;
        
        {
            GameEngine engine;
            EngineConfig config;
            REQUIRE(engine.initialize(config));
            
            window_ptr = engine.get_window();
            renderer_ptr = engine.get_renderer();
            
            REQUIRE(window_ptr != nullptr);
            REQUIRE(renderer_ptr != nullptr);
            
            // Engine goes out of scope here, should cleanup automatically
        }
        
        // Note: We can't directly test if SDL resources were freed
        // as SDL doesn't provide a way to check if a pointer is still valid
        // The test passes if no crashes occur during destruction
        SUCCEED("RAII cleanup completed without crashes");
    }
}

TEST_CASE("Memory safety with smart pointers", "[engine][memory][safety]") {
    SECTION("No global state variables exist") {
        // This is more of a design verification
        // The GameEngine class should not use any global variables
        GameEngine engine1;
        GameEngine engine2;
        
        EngineConfig config1;
        config1.window_title = "Engine 1";
        
        EngineConfig config2;
        config2.window_title = "Engine 2";
        
        // Both engines should be able to exist independently
        REQUIRE_FALSE(engine1.is_initialized());
        REQUIRE_FALSE(engine2.is_initialized());
        
        // Initialize first engine
        REQUIRE(engine1.initialize(config1));
        REQUIRE(engine1.is_initialized());
        
        // Second engine should also be able to initialize (SDL supports multiple init calls)
        // This demonstrates no global state interference
        REQUIRE(engine2.initialize(config2));
        REQUIRE(engine2.is_initialized());
        
        // Both engines should be independent
        REQUIRE(engine1.is_initialized());
        REQUIRE(engine2.is_initialized());
    }
}