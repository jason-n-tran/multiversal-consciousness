#include "InputManager.h"
#include <iostream>

InputManager::InputManager() {
    initialize_default_mappings();
}

void InputManager::initialize_default_mappings() {
    key_mappings_[SDLK_W] = InputAction::MOVE_UP;
    key_mappings_[SDLK_UP] = InputAction::MOVE_UP;
    key_mappings_[SDLK_S] = InputAction::MOVE_DOWN;
    key_mappings_[SDLK_DOWN] = InputAction::MOVE_DOWN;
    key_mappings_[SDLK_A] = InputAction::MOVE_LEFT;
    key_mappings_[SDLK_LEFT] = InputAction::MOVE_LEFT;
    key_mappings_[SDLK_D] = InputAction::MOVE_RIGHT;
    key_mappings_[SDLK_RIGHT] = InputAction::MOVE_RIGHT;
    
    key_mappings_[SDLK_E] = InputAction::INTERACT;
    key_mappings_[SDLK_SPACE] = InputAction::INTERACT;
    key_mappings_[SDLK_TAB] = InputAction::SWITCH_REALITY;
    key_mappings_[SDLK_ESCAPE] = InputAction::PAUSE;
    
    key_mappings_[SDLK_1] = InputAction::POSSESS_AGENT_1;
    key_mappings_[SDLK_2] = InputAction::POSSESS_AGENT_2;
    key_mappings_[SDLK_3] = InputAction::POSSESS_AGENT_3;
    key_mappings_[SDLK_4] = InputAction::POSSESS_AGENT_4;
    key_mappings_[SDLK_5] = InputAction::POSSESS_AGENT_5;
    key_mappings_[SDLK_6] = InputAction::POSSESS_AGENT_6;
    key_mappings_[SDLK_7] = InputAction::POSSESS_AGENT_7;
    key_mappings_[SDLK_8] = InputAction::POSSESS_AGENT_8;
    key_mappings_[SDLK_9] = InputAction::POSSESS_AGENT_9;
    
    for (const auto& [key, action] : key_mappings_) {
        action_states_[action] = InputState::RELEASED;
        previous_action_states_[action] = InputState::RELEASED;
    }
}

bool InputManager::process_event(const SDL_Event& event) {
    bool handled = false;
    
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN: {
            SDL_Keycode key = event.key.key;
            
            if (pressed_keys_.find(key) == pressed_keys_.end()) {
                pressed_keys_.insert(key);
                newly_pressed_keys_.insert(key);
                handled = true;
            }
            break;
        }
        
        case SDL_EVENT_KEY_UP: {
            SDL_Keycode key = event.key.key;
            
            if (pressed_keys_.find(key) != pressed_keys_.end()) {
                pressed_keys_.erase(key);
                newly_released_keys_.insert(key);
                handled = true;
            }
            break;
        }
        
        default:
            break;
    }
    
    return handled;
}

void InputManager::update(float delta_time) {
    previous_action_states_ = action_states_;
    
    update_action_states();
    
    trigger_callbacks(delta_time);
    
    newly_pressed_keys_.clear();
    newly_released_keys_.clear();
}

void InputManager::update_action_states() {
    for (const auto& [action, state] : action_states_) {
        InputState new_state = InputState::RELEASED;
        
        for (const auto& [key, mapped_action] : key_mappings_) {
            if (mapped_action == action && pressed_keys_.find(key) != pressed_keys_.end()) {
                InputState previous_state = previous_action_states_[action];
                if (previous_state == InputState::RELEASED) {
                    new_state = InputState::PRESSED; 
                } else {
                    new_state = InputState::HELD;
                }
                break; 
            }
        }
        
        action_states_[action] = new_state;
    }
}

void InputManager::trigger_callbacks(float delta_time) {
    for (const auto& [action, callbacks] : action_callbacks_) {
        InputState current_state = action_states_[action];
        InputState previous_state = previous_action_states_[action];
        
        if (current_state != previous_state || current_state != InputState::RELEASED) {
            for (const auto& callback : callbacks) {
                callback(action, current_state, delta_time);
            }
        }
    }
}

bool InputManager::is_action_active(InputAction action) const {
    auto it = action_states_.find(action);
    if (it != action_states_.end()) {
        return it->second == InputState::PRESSED || it->second == InputState::HELD;
    }
    return false;
}

bool InputManager::is_action_just_pressed(InputAction action) const {
    auto it = action_states_.find(action);
    if (it != action_states_.end()) {
        return it->second == InputState::PRESSED;
    }
    return false;
}

bool InputManager::is_action_just_released(InputAction action) const {
    auto current_it = action_states_.find(action);
    auto previous_it = previous_action_states_.find(action);
    
    if (current_it != action_states_.end() && previous_it != previous_action_states_.end()) {
        return current_it->second == InputState::RELEASED && 
               (previous_it->second == InputState::PRESSED || previous_it->second == InputState::HELD);
    }
    return false;
}

InputState InputManager::get_action_state(InputAction action) const {
    auto it = action_states_.find(action);
    if (it != action_states_.end()) {
        return it->second;
    }
    return InputState::RELEASED;
}

void InputManager::register_callback(InputAction action, InputCallback callback) {
    action_callbacks_[action].push_back(callback);
}

void InputManager::clear_callbacks(InputAction action) {
    action_callbacks_[action].clear();
}

void InputManager::clear_all_callbacks() {
    action_callbacks_.clear();
}

void InputManager::map_key(SDL_Keycode key, InputAction action) {
    key_mappings_[key] = action;
    
    // Initialize action state if not already present
    if (action_states_.find(action) == action_states_.end()) {
        action_states_[action] = InputState::RELEASED;
        previous_action_states_[action] = InputState::RELEASED;
    }
}

void InputManager::unmap_key(SDL_Keycode key) {
    key_mappings_.erase(key);
}

const std::unordered_set<SDL_Keycode>& InputManager::get_pressed_keys() const {
    return pressed_keys_;
}

const std::unordered_set<SDL_Keycode>& InputManager::get_newly_pressed_keys() const {
    return newly_pressed_keys_;
}

const std::unordered_set<SDL_Keycode>& InputManager::get_newly_released_keys() const {
    return newly_released_keys_;
}

bool InputManager::are_actions_active(const std::vector<InputAction>& actions) const {
    for (InputAction action : actions) {
        if (!is_action_active(action)) {
            return false;
        }
    }
    return true;
}

SDL_Keycode InputManager::get_key_for_action(InputAction action) const {
    for (const auto& [key, mapped_action] : key_mappings_) {
        if (mapped_action == action) {
            return key;
        }
    }
    return SDLK_UNKNOWN;
}

void InputManager::reset_states() {
    pressed_keys_.clear();
    newly_pressed_keys_.clear();
    newly_released_keys_.clear();
    
    for (auto& [action, state] : action_states_) {
        state = InputState::RELEASED;
    }
    for (auto& [action, state] : previous_action_states_) {
        state = InputState::RELEASED;
    }
}