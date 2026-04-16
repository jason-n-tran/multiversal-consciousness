#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

struct Transform {
    float x{0.0f}; 
    float y{0.0f}; 
    float rotation{0.0f}; 
    float scale_x{1.0f};  
    float scale_y{1.0f}; 
};

struct Renderable {
    std::string texture_id;                  
    SDL_Rect source_rect{0, 0, 32, 32};      
    float color_r{1.0f};                   
    float color_g{1.0f};                
    float color_b{1.0f};                  
    float color_a{1.0f};                      
    int layer{0};                       
};

struct Agent {
    uint8_t agent_number{1};    
    bool is_possessed{false};    
    float movement_speed{100.0f}; 
};

struct Inventory {
    std::unordered_set<std::string> items;         
    std::unordered_map<std::string, int> abilities;        
};

struct QuantumNode {
    std::string reality_a_item;   
    std::string reality_b_item;   
    bool is_activated{false};       
    float interaction_radius{32.0f}; 
};

struct Door {
    bool is_open{false};
    bool is_locked{false}; 
    std::string required_key;
    float animation_progress{0.0f};  
};

struct WaterLevel {
    float current_level{0.0f};      
    float target_level{0.0f};     
    float change_rate{50.0f};     
    bool is_draining{false};     
    bool is_filling{false};       
};

struct EnvironmentalSwitch {
    bool is_activated{false};   
    std::string target_entity_type; 
    std::string target_entity_id; 
    bool requires_agent_presence{true}; 
};