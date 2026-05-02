#pragma once

#include "System.h"
#include "CameraController.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <vector>
#include <array>
#include <memory>

struct BackgroundLayer {
    SDL_Texture* texture{nullptr};
    float parallax_factor{0.0f};
    bool is_tiled{false};
    float offset_x{0.0f};
    float offset_y{0.0f};
    float width{0.0f};
    float height{0.0f};
};

class BackgroundSystem : public IRenderSystem {
public:
    BackgroundSystem();
    ~BackgroundSystem() override;

    void update(float delta_time) override;
    void render(SDL_Renderer* renderer) override;
    void shutdown() override;

    void set_camera_controller(CameraController* camera_controller);
    void set_level_index(int level_index);
    
    void load_level_backgrounds(SDL_Renderer* renderer);

private:
    CameraController* camera_controller_{nullptr};
    int current_level_index_{-1};
    int loaded_level_index_{-2}; 
    
    static constexpr int NUM_BACKGROUND_FOLDERS = 4;
    
    std::vector<BackgroundLayer> layers_;
    
    void clear_textures();
};
