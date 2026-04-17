#pragma once

#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "Components.h"
#include <SDL3/SDL.h>
#include <optional>

struct CameraBounds {
    float min_x{-1000.0f};   
    float max_x{1000.0f};   
    float min_y{-1000.0f};    
    float max_y{1000.0f}; 
};

class CameraController {
private:
    float camera_x_{0.0f};                   
    float camera_y_{0.0f};             
    float target_x_{0.0f};              
    float target_y_{0.0f};                  
    
    std::optional<EntityID> target_entity_;   
    CameraBounds bounds_;                    
    
    float follow_speed_{5.0f};                
    float smoothing_factor_{0.1f};           
    
    int screen_width_{800};                 
    int screen_height_{600};           
    
    ComponentRegistry* component_registry_{nullptr}; 
    
    void apply_bounds(float& x, float& y) const;
    
    void update_target_position();
    
public:
    CameraController() = default;
    
    ~CameraController() = default;
    
    CameraController(const CameraController&) = delete;
    CameraController& operator=(const CameraController&) = delete;
    
    CameraController(CameraController&&) noexcept = default;
    CameraController& operator=(CameraController&&) noexcept = default;
    
    void initialize(ComponentRegistry& component_registry, int screen_width, int screen_height);
    
    void update(float delta_time);
    
    void set_target_entity(std::optional<EntityID> entity);
    
    std::optional<EntityID> get_target_entity() const;
    
    void set_position(float x, float y);
    
    void get_position(float& x, float& y) const;
    
    float get_x() const { return camera_x_; }
    
    float get_y() const { return camera_y_; }
    
    void set_bounds(const CameraBounds& bounds);
    
    const CameraBounds& get_bounds() const;
    
    void set_follow_speed(float speed);
    
    float get_follow_speed() const { return follow_speed_; }
    
    void set_smoothing_factor(float factor);
    
    float get_smoothing_factor() const { return smoothing_factor_; }
    
    void world_to_screen(float world_x, float world_y, int& screen_x, int& screen_y) const;
    
    void screen_to_world(int screen_x, int screen_y, float& world_x, float& world_y) const;
    
    void update_screen_dimensions(int width, int height);
};