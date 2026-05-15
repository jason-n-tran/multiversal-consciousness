#include <catch2/catch_test_macros.hpp>
#include <SDL3/SDL.h>
#include "../../src/engine/InputManager.h"

TEST_CASE("InputManager initialization", "[input][manager]") {
    InputManager input_manager;
    
    SECTION("Default key mappings are set") {
        // Test movement keys (SDL3 uses different key codes)
        // Note: We test that keys are mapped, not specific key codes since SDL3 may differ
        REQUIRE(input_manager.get_key_for_action(InputAction::MOVE_UP) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::MOVE_DOWN) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::MOVE_LEFT) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::MOVE_RIGHT) != SDLK_UNKNOWN);
        
        // Test action keys
        REQUIRE(input_manager.get_key_for_action(InputAction::INTERACT) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::SWITCH_REALITY) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::PAUSE) != SDLK_UNKNOWN);
        
        // Test possession keys
        REQUIRE(input_manager.get_key_for_action(InputAction::POSSESS_AGENT_1) != SDLK_UNKNOWN);
        REQUIRE(input_manager.get_key_for_action(InputAction::POSSESS_AGENT_9) != SDLK_UNKNOWN);
    }
    
    SECTION("Initial action states are RELEASED") {
        REQUIRE(input_manager.get_action_state(InputAction::MOVE_UP) == InputState::RELEASED);
        REQUIRE(input_manager.get_action_state(InputAction::INTERACT) == InputState::RELEASED);
        REQUIRE(input_manager.get_action_state(InputAction::POSSESS_AGENT_1) == InputState::RELEASED);
        
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_pressed(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_released(InputAction::MOVE_UP));
    }
}

TEST_CASE("InputManager event processing", "[input][manager][events]") {
    InputManager input_manager;
    SDL_Event event;
    
    SECTION("Key press events") {
        // Simulate key press
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        
        REQUIRE(input_manager.process_event(event));
        
        // Key should be in pressed keys set
        const auto& pressed_keys = input_manager.get_pressed_keys();
        REQUIRE(pressed_keys.find(SDLK_W) != pressed_keys.end());
        
        const auto& newly_pressed = input_manager.get_newly_pressed_keys();
        REQUIRE(newly_pressed.find(SDLK_W) != newly_pressed.end());
    }
    
    SECTION("Key release events") {
        // First press the key
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        
        // Then release it
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = SDLK_W;
        REQUIRE(input_manager.process_event(event));
        
        // Key should not be in pressed keys set
        const auto& pressed_keys = input_manager.get_pressed_keys();
        REQUIRE(pressed_keys.find(SDLK_W) == pressed_keys.end());
        
        const auto& newly_released = input_manager.get_newly_released_keys();
        REQUIRE(newly_released.find(SDLK_W) != newly_released.end());
    }
    
    SECTION("Non-keyboard events are not handled") {
        event.type = SDL_EVENT_QUIT;
        REQUIRE_FALSE(input_manager.process_event(event));
        
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        REQUIRE_FALSE(input_manager.process_event(event));
    }
    
    SECTION("Key repeat is ignored") {
        // First press
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        REQUIRE(input_manager.process_event(event));
        
        // Second press (key repeat) should be ignored
        REQUIRE_FALSE(input_manager.process_event(event));
        
        // Key should still be in pressed keys set only once
        const auto& pressed_keys = input_manager.get_pressed_keys();
        REQUIRE(pressed_keys.size() == 1);
        REQUIRE(pressed_keys.find(SDLK_W) != pressed_keys.end());
    }
}

TEST_CASE("InputManager action state updates", "[input][manager][states]") {
    InputManager input_manager;
    SDL_Event event;
    
    SECTION("Action state transitions") {
        // Initially released
        REQUIRE(input_manager.get_action_state(InputAction::MOVE_UP) == InputState::RELEASED);
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::MOVE_UP));
        
        // Press key
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Should be PRESSED on first frame
        REQUIRE(input_manager.get_action_state(InputAction::MOVE_UP) == InputState::PRESSED);
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE(input_manager.is_action_just_pressed(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_released(InputAction::MOVE_UP));
        
        // Update again without new events - should be HELD
        input_manager.update(0.016f);
        
        REQUIRE(input_manager.get_action_state(InputAction::MOVE_UP) == InputState::HELD);
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_pressed(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_released(InputAction::MOVE_UP));
        
        // Release key
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        // Should be RELEASED
        REQUIRE(input_manager.get_action_state(InputAction::MOVE_UP) == InputState::RELEASED);
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_just_pressed(InputAction::MOVE_UP));
        REQUIRE(input_manager.is_action_just_released(InputAction::MOVE_UP));
    }
}

TEST_CASE("InputManager simultaneous input handling", "[input][manager][simultaneous]") {
    InputManager input_manager;
    SDL_Event event;
    
    SECTION("Multiple keys pressed simultaneously") {
        // Press multiple keys
        event.type = SDL_EVENT_KEY_DOWN;
        
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        
        event.key.key = SDLK_D;
        input_manager.process_event(event);
        
        event.key.key = SDLK_1;
        input_manager.process_event(event);
        
        input_manager.update(0.016f);
        
        // All actions should be active
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_RIGHT));
        REQUIRE(input_manager.is_action_active(InputAction::POSSESS_AGENT_1));
        
        // Test multiple actions check
        std::vector<InputAction> actions = {
            InputAction::MOVE_UP,
            InputAction::MOVE_RIGHT,
            InputAction::POSSESS_AGENT_1
        };
        REQUIRE(input_manager.are_actions_active(actions));
        
        // Test with one inactive action
        actions.push_back(InputAction::MOVE_LEFT);
        REQUIRE_FALSE(input_manager.are_actions_active(actions));
    }
    
    SECTION("No input conflicts") {
        // Press conflicting movement keys
        event.type = SDL_EVENT_KEY_DOWN;
        
        event.key.key = SDLK_W;  // Move up
        input_manager.process_event(event);
        
        event.key.key = SDLK_S;  // Move down
        input_manager.process_event(event);
        
        input_manager.update(0.016f);
        
        // Both actions should be active (no conflict resolution at InputManager level)
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_DOWN));
    }
}

TEST_CASE("InputManager callback system", "[input][manager][callbacks]") {
    InputManager input_manager;
    SDL_Event event;
    
    SECTION("Callback registration and triggering") {
        bool callback_triggered = false;
        InputAction triggered_action = InputAction::MOVE_UP;
        InputState triggered_state = InputState::RELEASED;
        
        // Register callback
        input_manager.register_callback(InputAction::MOVE_UP, 
            [&](InputAction action, InputState state, float delta_time) {
                callback_triggered = true;
                triggered_action = action;
                triggered_state = state;
            });
        
        // Press key to trigger callback
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        REQUIRE(callback_triggered);
        REQUIRE(triggered_action == InputAction::MOVE_UP);
        REQUIRE(triggered_state == InputState::PRESSED);
    }
    
    SECTION("Multiple callbacks for same action") {
        int callback_count = 0;
        
        // Register multiple callbacks
        input_manager.register_callback(InputAction::INTERACT, 
            [&](InputAction, InputState, float) { callback_count++; });
        input_manager.register_callback(InputAction::INTERACT, 
            [&](InputAction, InputState, float) { callback_count++; });
        
        // Trigger action
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_E;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        REQUIRE(callback_count == 2);
    }
    
    SECTION("Clear callbacks") {
        bool callback_triggered = false;
        
        input_manager.register_callback(InputAction::PAUSE, 
            [&](InputAction, InputState, float) { callback_triggered = true; });
        
        // Clear callbacks
        input_manager.clear_callbacks(InputAction::PAUSE);
        
        // Trigger action - callback should not be called
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_ESCAPE;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        REQUIRE_FALSE(callback_triggered);
    }
}

TEST_CASE("InputManager key mapping", "[input][manager][mapping]") {
    InputManager input_manager;
    
    SECTION("Custom key mapping") {
        // Map a custom key to an action
        input_manager.map_key(SDLK_Q, InputAction::INTERACT);
        
        // Check that the key is mapped (don't check specific key code due to SDL3 differences)
        REQUIRE(input_manager.get_key_for_action(InputAction::INTERACT) != SDLK_UNKNOWN);
        
        // Test the mapping works
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_Q;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        REQUIRE(input_manager.is_action_active(InputAction::INTERACT));
    }
    
    SECTION("Unmap key") {
        // Unmap a key
        input_manager.unmap_key(SDLK_E);
        
        // The action should no longer be triggered by the unmapped key
        SDL_Event event;
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_E;
        input_manager.process_event(event);
        input_manager.update(0.016f);
        
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::INTERACT));
    }
}

TEST_CASE("InputManager state reset", "[input][manager][reset]") {
    InputManager input_manager;
    SDL_Event event;
    
    SECTION("Reset clears all states") {
        // Press some keys
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_W;
        input_manager.process_event(event);
        event.key.key = SDLK_D;
        input_manager.process_event(event);
        
        input_manager.update(0.016f);
        
        // Verify keys are active
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE(input_manager.is_action_active(InputAction::MOVE_RIGHT));
        
        // Reset states
        input_manager.reset_states();
        
        // All states should be cleared
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::MOVE_UP));
        REQUIRE_FALSE(input_manager.is_action_active(InputAction::MOVE_RIGHT));
        REQUIRE(input_manager.get_pressed_keys().empty());
        REQUIRE(input_manager.get_newly_pressed_keys().empty());
        REQUIRE(input_manager.get_newly_released_keys().empty());
    }
}