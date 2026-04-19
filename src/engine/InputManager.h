#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include <cstdint>

enum class InputAction : uint8_t {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    INTERACT,
    SWITCH_REALITY,
    PAUSE,
    POSSESS_AGENT_1,
    POSSESS_AGENT_2,
    POSSESS_AGENT_3,
    POSSESS_AGENT_4,
    POSSESS_AGENT_5,
    POSSESS_AGENT_6,
    POSSESS_AGENT_7,
    POSSESS_AGENT_8,
    POSSESS_AGENT_9
};

enum class InputState : uint8_t {
    RELEASED,
    PRESSED,
    HELD
};

using InputCallback = std::function<void(InputAction action, InputState state, float delta_time)>;

class InputManager {
private:
    std::unordered_map<SDL_Keycode, InputAction> key_mappings_;
    
    std::unordered_map<InputAction, InputState> action_states_;
    
    std::unordered_map<InputAction, InputState> previous_action_states_;
    
    std::unordered_map<InputAction, std::vector<InputCallback>> action_callbacks_;
    
    std::unordered_set<SDL_Keycode> pressed_keys_;
    
    std::unordered_set<SDL_Keycode> newly_pressed_keys_;
    
    std::unordered_set<SDL_Keycode> newly_released_keys_;
    
    void initialize_default_mappings();
    
    void update_action_states();
    
    void trigger_callbacks(float delta_time);
    
public:
    InputManager();
    
    ~InputManager() = default;
    
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    InputManager(InputManager&&) noexcept = default;
    InputManager& operator=(InputManager&&) noexcept = default;
    
    bool process_event(const SDL_Event& event);
    
    void update(float delta_time);
    
    bool is_action_active(InputAction action) const;
    
    bool is_action_just_pressed(InputAction action) const;
    
    bool is_action_just_released(InputAction action) const;
    
    InputState get_action_state(InputAction action) const;
    
    void register_callback(InputAction action, InputCallback callback);
    
    void clear_callbacks(InputAction action);
    
    void clear_all_callbacks();
    
    void map_key(SDL_Keycode key, InputAction action);
    
    void unmap_key(SDL_Keycode key);
    
    const std::unordered_set<SDL_Keycode>& get_pressed_keys() const;
    
    const std::unordered_set<SDL_Keycode>& get_newly_pressed_keys() const;
    
    const std::unordered_set<SDL_Keycode>& get_newly_released_keys() const;
    
    bool are_actions_active(const std::vector<InputAction>& actions) const;
    
    SDL_Keycode get_key_for_action(InputAction action) const;
    
    void reset_states();
};