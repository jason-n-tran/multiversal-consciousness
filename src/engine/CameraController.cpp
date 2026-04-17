#include "CameraController.h"
#include <algorithm>
#include <cmath>
#include <iostream>

void CameraController::initialize(ComponentRegistry& component_registry, int screen_width, int screen_height) {
    component_registry_ = &component_registry;
    screen_width_ = screen_width;
    screen_height_ = screen_height;
    
    std::cout << "CameraController initialized with screen dimensions: " 
              << screen_width << "x" << screen_height << std::endl;
}

void CameraController::update(float delta_time) {
    update_target_position();
    
    if (smoothing_factor_ > 0.0f) {
        float lerp_factor = 1.0f - std::exp(-follow_speed_ * delta_time);
        lerp_factor = std::clamp(lerp_factor, 0.0f, 1.0f);
        
        camera_x_ += (target_x_ - camera_x_) * lerp_factor;
        camera_y_ += (target_y_ - camera_y_) * lerp_factor;
    } else {
        camera_x_ = target_x_;
        camera_y_ = target_y_;
    }
    apply_bounds(camera_x_, camera_y_);
}

void CameraController::update_target_position() {
    if (!target_entity_.has_value() || !component_registry_) {
        return;
    }
    
    const Transform* transform = component_registry_->get_component<Transform>(target_entity_.value());
    if (transform) {
        target_x_ = transform->x;
        target_y_ = transform->y;
    }
}

void CameraController::apply_bounds(float& x, float& y) const {
    float half_screen_width = screen_width_ * 0.5f;
    float half_screen_height = screen_height_ * 0.5f;
    
    x = std::clamp(x, bounds_.min_x + half_screen_width, bounds_.max_x - half_screen_width);
    y = std::clamp(y, bounds_.min_y + half_screen_height, bounds_.max_y - half_screen_height);
}

void CameraController::set_target_entity(std::optional<EntityID> entity) {
    target_entity_ = entity;
    
    if (entity.has_value()) {
        std::cout << "Camera now following Entity ID: " << entity.value() << std::endl;
        
        update_target_position();
    } else {
        std::cout << "Camera stopped following entity" << std::endl;
    }
}

std::optional<EntityID> CameraController::get_target_entity() const {
    return target_entity_;
}

void CameraController::set_position(float x, float y) {
    camera_x_ = x;
    camera_y_ = y;
    target_x_ = x;
    target_y_ = y;
    
    // Apply bounds
    apply_bounds(camera_x_, camera_y_);
    apply_bounds(target_x_, target_y_);
}

void CameraController::get_position(float& x, float& y) const {
    x = camera_x_;
    y = camera_y_;
}

void CameraController::set_bounds(const CameraBounds& bounds) {
    bounds_ = bounds;
    
    apply_bounds(camera_x_, camera_y_);
    apply_bounds(target_x_, target_y_);
}

const CameraBounds& CameraController::get_bounds() const {
    return bounds_;
}

void CameraController::set_follow_speed(float speed) {
    follow_speed_ = std::max(0.1f, speed);
}

void CameraController::set_smoothing_factor(float factor) {
    smoothing_factor_ = std::clamp(factor, 0.0f, 1.0f);
}

void CameraController::world_to_screen(float world_x, float world_y, int& screen_x, int& screen_y) const {
    screen_x = static_cast<int>((world_x - camera_x_) + (screen_width_ * 0.5f));
    screen_y = static_cast<int>((world_y - camera_y_) + (screen_height_ * 0.5f));
}

void CameraController::screen_to_world(int screen_x, int screen_y, float& world_x, float& world_y) const {
    world_x = (screen_x - (screen_width_ * 0.5f)) + camera_x_;
    world_y = (screen_y - (screen_height_ * 0.5f)) + camera_y_;
}

void CameraController::update_screen_dimensions(int width, int height) {
    screen_width_ = width;
    screen_height_ = height;
    
    apply_bounds(camera_x_, camera_y_);
    apply_bounds(target_x_, target_y_);
    
    std::cout << "Camera screen dimensions updated to: " << width << "x" << height << std::endl;
}