#include "BackgroundSystem.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <filesystem>
#include <cmath>

BackgroundSystem::BackgroundSystem() = default;

BackgroundSystem::~BackgroundSystem() {
    clear_textures();
}

void BackgroundSystem::shutdown() {
    clear_textures();
}

void BackgroundSystem::set_camera_controller(CameraController* camera_controller) {
    camera_controller_ = camera_controller;
}

void BackgroundSystem::set_level_index(int level_index) {
    if (current_level_index_ != level_index) {
        current_level_index_ = level_index;
    }
}

void BackgroundSystem::clear_textures() {
    for (auto& layer : layers_) {
        if (layer.texture) {
            SDL_DestroyTexture(layer.texture);
            layer.texture = nullptr;
        }
    }
    layers_.clear();
}

void BackgroundSystem::load_level_backgrounds(SDL_Renderer* renderer) {
    if (!renderer) return;

    clear_textures();
    loaded_level_index_ = current_level_index_;

    int folder_index = (std::max(0, current_level_index_) % NUM_BACKGROUND_FOLDERS) + 1;
    std::string base_path = "assets/" + std::to_string(folder_index) + " background/";

    std::cout << "Loading backgrounds from: " << base_path << std::endl;

    {
        std::string path = base_path + "1.png";
        SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
        if (tex) {
            BackgroundLayer layer;
            layer.texture = tex;
            layer.parallax_factor = 0.05f; 
            layer.is_tiled = true;
            SDL_GetTextureSize(tex, &layer.width, &layer.height);
            layers_.push_back(layer);
            std::cout << "Loaded background layer: " << path << std::endl;
        } else {
            std::cerr << "Failed to load background layer: " << path << " Error: " << SDL_GetError() << std::endl;
        }
    }

    float start_positions[3][2] = {
        {200.0f, 100.0f}, 
        {600.0f, 300.0f}, 
        {1000.0f, 150.0f}  
    };

    float parallax_factors[3] = {0.1f, 0.2f, 0.3f}; 

    for (int i = 0; i < 3; ++i) {
        std::string filename = std::to_string(i + 2) + ".png";
        std::string path = base_path + filename;
        
        SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
        if (tex) {
            BackgroundLayer layer;
            layer.texture = tex;
            layer.parallax_factor = parallax_factors[i];
            layer.is_tiled = false;
            layer.offset_x = start_positions[i][0];
            layer.offset_y = start_positions[i][1];
            SDL_GetTextureSize(tex, &layer.width, &layer.height);
            layers_.push_back(layer);
            std::cout << "Loaded planet layer: " << path << std::endl;
        } else {
        }
    }
}

void BackgroundSystem::update(float delta_time) {
    (void)delta_time; 
}

void BackgroundSystem::render(SDL_Renderer* renderer) {
    if (!renderer) return;

    if (loaded_level_index_ != current_level_index_) {
        load_level_backgrounds(renderer);
    }
    
    if (layers_.empty()) return;

    float cam_x = 0.0f;
    float cam_y = 0.0f;
    int screen_w = 800; 
    int screen_h = 600; 

    if (camera_controller_) {
        cam_x = camera_controller_->get_x();
        cam_y = camera_controller_->get_y();
        screen_w = camera_controller_->get_screen_width();
        screen_h = camera_controller_->get_screen_height();
    }
    
    for (const auto& layer : layers_) {
        if (!layer.texture) continue;

        if (layer.is_tiled) {
            float p_x = cam_x * layer.parallax_factor;
            float p_y = cam_y * layer.parallax_factor;
            
            float tile_offset_x = fmod(p_x, layer.width);
            float tile_offset_y = fmod(p_y, layer.height);
            
            if (tile_offset_x < 0) tile_offset_x += layer.width;
            if (tile_offset_y < 0) tile_offset_y += layer.height;

            int start_x = static_cast<int>(-tile_offset_x);
            int start_y = static_cast<int>(-tile_offset_y);
            
            for (float x = static_cast<float>(start_x); x < screen_w; x += layer.width) {
                 for (float y = static_cast<float>(start_y); y < screen_h; y += layer.height) {
                     SDL_FRect dst_rect = {
                         x, 
                         y, 
                         layer.width, 
                         layer.height
                     };
                     SDL_RenderTexture(renderer, layer.texture, nullptr, &dst_rect);
                 }
            }
        } else {
            float relative_x = layer.offset_x - (cam_x * layer.parallax_factor);
            float relative_y = layer.offset_y - (cam_y * layer.parallax_factor);
            
            float draw_x = relative_x;
            float draw_y = relative_y;
            
            if (draw_x + layer.width > 0 && draw_x < screen_w &&
                draw_y + layer.height > 0 && draw_y < screen_h) {
                
                SDL_FRect dst_rect = {
                    draw_x,
                    draw_y,
                    layer.width,
                    layer.height
                };
                SDL_RenderTexture(renderer, layer.texture, nullptr, &dst_rect);
            }
        }
    }
}
