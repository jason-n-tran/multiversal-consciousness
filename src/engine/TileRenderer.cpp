#include "TileRenderer.h"
#include "Components.h"
#include <iostream>
#include <algorithm>

Tile* TileMap::get_tile(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height || tiles[y][x].empty()) {
        return nullptr;
    }
    return &tiles[y][x].back();
}

const Tile* TileMap::get_tile(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height || tiles[y][x].empty()) {
        return nullptr;
    }
    return &tiles[y][x].back();
}

std::vector<Tile>* TileMap::get_tile_stack(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return nullptr;
    }
    return &tiles[y][x];
}

const std::vector<Tile>* TileMap::get_tile_stack(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return nullptr;
    }
    return &tiles[y][x];
}

bool TileMap::set_tile(int x, int y, const Tile& tile) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }
    
    auto& stack = tiles[y][x];
    
    for (auto& existing : stack) {
        if (existing.layer == tile.layer) {
            existing = tile;
            return true;
        }
    }
    
    stack.push_back(tile);
    std::sort(stack.begin(), stack.end(), [](const Tile& a, const Tile& b) {
        return a.layer < b.layer;
    });
    
    return true;
}

void TileMap::remove_tiles_by_id_range(int x, int y, int min_id, int max_id) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    
    auto& stack = tiles[y][x];
    stack.erase(std::remove_if(stack.begin(), stack.end(), [min_id, max_id](const Tile& t) {
        return t.texture_id >= min_id && t.texture_id <= max_id;
    }), stack.end());
}

void TileMap::initialize(int w, int h, const Tile& default_tile) {
    width = w;
    height = h;
    tiles.clear();
    tiles.resize(height);
    for (int y = 0; y < height; ++y) {
        tiles[y].resize(width);
        for (int x = 0; x < width; ++x) {
            tiles[y][x].push_back(default_tile);
        }
    }
}

void TileMap::clear() {
    tiles.clear();
    width = 0;
    height = 0;
}

void Camera::world_to_screen(float world_x, float world_y, int& screen_x, int& screen_y) const {
    screen_x = static_cast<int>((world_x - x) * zoom + viewport_width / 2.0f);
    screen_y = static_cast<int>((world_y - y) * zoom + viewport_height / 2.0f);
}

void Camera::screen_to_world(int screen_x, int screen_y, float& world_x, float& world_y) const {
    world_x = (screen_x - viewport_width / 2.0f) / zoom + x;
    world_y = (screen_y - viewport_height / 2.0f) / zoom + y;
}

bool Camera::is_visible(float world_x, float world_y, float width, float height) const {
    float left = x - (viewport_width / 2.0f) / zoom;
    float right = x + (viewport_width / 2.0f) / zoom;
    float top = y - (viewport_height / 2.0f) / zoom;
    float bottom = y + (viewport_height / 2.0f) / zoom;
    
    return !(world_x + width < left || world_x > right || 
             world_y + height < top || world_y > bottom);
}

TileRenderer::TileRenderer(const EngineConfig& config)
    : tile_size_(config.tile_size)
    , render_scale_(config.render_scale)
    , show_grid_(false)
    , grid_color_{0.3f, 0.3f, 0.3f, 0.5f}
    , show_reality_indicator_(true)
    , visible_tiles_rendered_(0) {
    
    tile_map_ = std::make_unique<TileMap>();
    
    camera_.viewport_width = config.window_width;
    camera_.viewport_height = config.window_height;
}

void TileRenderer::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    entity_manager_ = &entity_manager;
    component_registry_ = &component_registry;
    
    std::cout << "TileRenderer initialized with tile size: " << tile_size_ 
              << ", render scale: " << render_scale_ << std::endl;
}

void TileRenderer::update(float delta_time) {
    if (camera_controller_) {
        camera_.x = camera_controller_->get_x();
        camera_.y = camera_controller_->get_y();
        // Note: zoom is not available from CameraController, keep current zoom
    }
    
    if (tile_map_ && component_registry_) {
        for (int y = 0; y < tile_map_->height; ++y) {
            for (int x = 0; x < tile_map_->width; ++x) {
                tile_map_->remove_tiles_by_id_range(x, y, 5, 8);
            }
        }
        const auto* wall_container = component_registry_->get_all_components<Wall>();
        if (wall_container) {
            for (EntityID entity : wall_container->get_entities()) {
                if (!entity_manager_->is_valid(entity)) {
                    continue;
                }
                const auto* wall = component_registry_->get_component<Wall>(entity);
                const auto* transform = component_registry_->get_component<Transform>(entity);
                
                if (wall && transform && wall->wall_type == "tree") {
                    int tile_x = static_cast<int>((transform->x - wall->width * 0.5f) / 32.0f);
                    int tile_y = static_cast<int>((transform->y - wall->height * 0.5f) / 32.0f);
                    int tile_width = static_cast<int>(wall->width / 32.0f);
                    int tile_height = static_cast<int>(wall->height / 32.0f);

                    for (int y = tile_y; y < tile_y + tile_height; ++y) {
                        for (int x = tile_x; x < tile_x + tile_width; ++x) {
                            Tile tile;
                            tile.texture_id = 5; 
                            tile.color = {1.0f, 1.0f, 1.0f, 1.0f};
                            tile.layer = 2; 
                            tile_map_->set_tile(x, y, tile);
                        }
                    }
                }
            }
        }

        const auto* door_container = component_registry_->get_all_components<Door>();
        if (door_container) {
            for (EntityID entity : door_container->get_entities()) {
                if (!entity_manager_->is_valid(entity)) {
                    continue;
                }
                const auto* door = component_registry_->get_component<Door>(entity);
                const auto* transform = component_registry_->get_component<Transform>(entity);
                
                if (door && transform) {
                    int tile_x = static_cast<int>((transform->x - 16.0f) / 32.0f);
                    int tile_y = static_cast<int>((transform->y - 16.0f) / 32.0f);
                    
                    Tile tile;
                    tile.texture_id = 6; 
                    tile.layer = 2; 
                    if (door->is_open) {
                        tile.color = {1.0f, 1.0f, 1.0f, 0.2f}; 
                    } else {
                        tile.color = {1.0f, 1.0f, 1.0f, 1.0f}; 
                    }
                    tile_map_->set_tile(tile_x, tile_y, tile);
                }
            }
        }

        const auto* water_container = component_registry_->get_all_components<WaterLevel>();
        if (water_container) {
            for (EntityID entity : water_container->get_entities()) {
                if (!entity_manager_->is_valid(entity)) {
                    continue;
                }
                const auto* water = component_registry_->get_component<WaterLevel>(entity);
                const auto* transform = component_registry_->get_component<Transform>(entity);
                const auto* bbox = component_registry_->get_component<BoundingBoxComponent>(entity);
                
                if (water && transform) {
                    int tile_x = static_cast<int>((transform->x - 16.0f) / 32.0f);
                    int tile_y = static_cast<int>((transform->y - 16.0f) / 32.0f);
                    
                    Tile tile;
                    tile.texture_id = 7; 
                    tile.layer = 1; 
                    bool is_solid_water = (bbox && bbox->is_solid);
                    
                    if (is_solid_water) {
                        tile.color = {0.7f, 0.9f, 1.0f, 1.0f}; 
                    } else if (water->target_level < 10.0f || (water->is_draining && water->current_level < 10.0f)) {
                        tile.color = {1.0f, 1.0f, 1.0f, 0.2f};
                    } else {
                        tile.color = {1.0f, 1.0f, 1.0f, 0.8f};
                    }
                    tile_map_->set_tile(tile_x, tile_y, tile);
                }
            }
        }

        const auto* switch_container = component_registry_->get_all_components<EnvironmentalSwitch>();
        if (switch_container) {
            for (EntityID entity : switch_container->get_entities()) {
                if (!entity_manager_->is_valid(entity)) {
                    continue;
                }
                const auto* env_switch = component_registry_->get_component<EnvironmentalSwitch>(entity);
                const auto* transform = component_registry_->get_component<Transform>(entity);
                
                if (env_switch && transform) {
                    int tile_x = static_cast<int>((transform->x - 16.0f) / 32.0f);
                    int tile_y = static_cast<int>((transform->y - 16.0f) / 32.0f);
                    
                    Tile tile;
                    tile.texture_id = 8; 
                    tile.layer = 1; 
                    if (env_switch->is_activated) {
                        tile.color = {0.0f, 1.0f, 0.0f, 1.0f}; 
                    } else {
                        tile.color = {1.0f, 1.0f, 1.0f, 1.0f}; 
                    }
                    tile_map_->set_tile(tile_x, tile_y, tile);
                }
            }
        }
    }
    
    (void)delta_time; 
}

void TileRenderer::render(SDL_Renderer* renderer) {
    if (!tile_map_ || !renderer) {
        return;
    }
    
    visible_tiles_rendered_ = 0;

    Reality current_reality = Reality::A;
    if (reality_manager_) {
        current_reality = reality_manager_->get_current_reality();
    }

    if (camera_controller_) {
        camera_.x = camera_controller_->get_x();
        camera_.y = camera_controller_->get_y();
        camera_.viewport_width = camera_controller_->get_screen_width();
        camera_.viewport_height = camera_controller_->get_screen_height();
    }
    
    float world_left, world_top, world_right, world_bottom;
    if (camera_controller_) {
        camera_controller_->screen_to_world(0, 0, world_left, world_top);
        camera_controller_->screen_to_world(camera_controller_->get_screen_width(), camera_controller_->get_screen_height(), world_right, world_bottom);
    } else {
        camera_.screen_to_world(0, 0, world_left, world_top);
        camera_.screen_to_world(camera_.viewport_width, camera_.viewport_height, world_right, world_bottom);
    }
    
    int tile_left = std::max(0, static_cast<int>(world_left / tile_size_) - 1);
    int tile_top = std::max(0, static_cast<int>(world_top / tile_size_) - 1);
    int tile_right = std::min(tile_map_->width - 1, static_cast<int>(world_right / tile_size_) + 1);
    int tile_bottom = std::min(tile_map_->height - 1, static_cast<int>(world_bottom / tile_size_) + 1);
    
    for (int layer = 0; layer <= 10; ++layer) {
        for (int y = tile_top; y <= tile_bottom; ++y) {
            for (int x = tile_left; x <= tile_right; ++x) {
                const std::vector<Tile>* stack = tile_map_->get_tile_stack(x, y);
                if (!stack) {
                    continue;
                }
                
                for (const auto& tile : *stack) {
                    if (!tile.visible || tile.layer != layer) {
                        continue;
                    }
                    
                    float world_x = x * tile_size_;
                    float world_y = y * tile_size_;
                    
                    int screen_x, screen_y;
                    if (camera_controller_) {
                        camera_controller_->world_to_screen(world_x, world_y, screen_x, screen_y);
                    } else {
                        camera_.world_to_screen(world_x, world_y, screen_x, screen_y);
                    }
                    
                    float zoom = camera_controller_ ? 1.0f : camera_.zoom; 
                    int scaled_size = static_cast<int>(tile_size_ * zoom * render_scale_);
                    
                    SDL_FRect dest_rect{
                        static_cast<float>(screen_x),
                        static_cast<float>(screen_y),
                        static_cast<float>(scaled_size),
                        static_cast<float>(scaled_size)
                    };
                    
                    int texture_id;
                    SDL_FColor color;
                    get_reality_visuals(tile, current_reality, texture_id, color);
                    
                    if (texture_id > 0) {
                        auto texture_it = textures_.find(texture_id);
                        if (texture_it != textures_.end()) {
                            SDL_SetTextureColorMod(texture_it->second.get(), 
                                                 static_cast<Uint8>(color.r * 255),
                                                 static_cast<Uint8>(color.g * 255),
                                                 static_cast<Uint8>(color.b * 255));
                            SDL_SetTextureAlphaMod(texture_it->second.get(), 
                                                 static_cast<Uint8>(color.a * 255));
                            
                            SDL_RenderTexture(renderer, texture_it->second.get(), 
                                            &tile.source_rect, &dest_rect);
                        }
                    } else {
                        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
                        SDL_RenderFillRect(renderer, &dest_rect);
                    }
                    
                    visible_tiles_rendered_++;
                }
            }
        }
    }
    
    if (show_grid_) {
        SDL_SetRenderDrawColorFloat(renderer, grid_color_.r, grid_color_.g, 
                                  grid_color_.b, grid_color_.a);
        
        for (int x = tile_left; x <= tile_right + 1; ++x) {
            float world_x = x * tile_size_;
            int screen_x, screen_y_top, screen_y_bottom;
            if (camera_controller_) {
                camera_controller_->world_to_screen(world_x, tile_top * tile_size_, screen_x, screen_y_top);
                camera_controller_->world_to_screen(world_x, (tile_bottom + 1) * tile_size_, screen_x, screen_y_bottom);
            } else {
                camera_.world_to_screen(world_x, tile_top * tile_size_, screen_x, screen_y_top);
                camera_.world_to_screen(world_x, (tile_bottom + 1) * tile_size_, screen_x, screen_y_bottom);
            }
            SDL_RenderLine(renderer, screen_x, screen_y_top, screen_x, screen_y_bottom);
        }
        
        for (int y = tile_top; y <= tile_bottom + 1; ++y) {
            float world_y = y * tile_size_;
            int screen_x_left, screen_x_right, screen_y;
            if (camera_controller_) {
                camera_controller_->world_to_screen(tile_left * tile_size_, world_y, screen_x_left, screen_y);
                camera_controller_->world_to_screen((tile_right + 1) * tile_size_, world_y, screen_x_right, screen_y);
            } else {
                camera_.world_to_screen(tile_left * tile_size_, world_y, screen_x_left, screen_y);
                camera_.world_to_screen((tile_right + 1) * tile_size_, world_y, screen_x_right, screen_y);
            }
            SDL_RenderLine(renderer, screen_x_left, screen_y, screen_x_right, screen_y);
        }
    }

    if (show_reality_indicator_) {
        render_reality_indicator(renderer, current_reality);
    }
}

void TileRenderer::shutdown() {
    textures_.clear();
    tile_map_.reset();
    std::cout << "TileRenderer shutdown complete" << std::endl;
}

bool TileRenderer::load_texture(int texture_id, const std::string& filename, SDL_Renderer* renderer) {
    if (!renderer) {
        std::cerr << "Cannot load texture: renderer is null" << std::endl;
        return false;
    }
    
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
    if (!texture) {
        std::cerr << "Failed to load texture: " << filename << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    textures_[texture_id] = TexturePtr(texture);
    std::cout << "Loaded texture ID " << texture_id << " from " << filename << std::endl;
    return true;
}

bool TileRenderer::create_solid_texture(int texture_id, SDL_FColor color, int width, int height, SDL_Renderer* renderer) {
    if (!renderer) {
        std::cerr << "Cannot create texture: renderer is null" << std::endl;
        return false;
    }
    
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        std::cerr << "Failed to create solid texture: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_Texture* previous_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, previous_target);
    
    textures_[texture_id] = TexturePtr(texture);
    std::cout << "Created solid texture ID " << texture_id << " (" << width << "x" << height << ")" << std::endl;
    return true;
}

void TileRenderer::set_tile_map(std::unique_ptr<TileMap> tile_map) {
    tile_map_ = std::move(tile_map);
}

void TileRenderer::set_camera_position(float x, float y) {
    camera_.x = x;
    camera_.y = y;
}

void TileRenderer::set_camera_zoom(float zoom) {
    camera_.zoom = std::max(0.1f, std::min(10.0f, zoom));  // Clamp zoom between 0.1x and 10x
}

void TileRenderer::update_config(const EngineConfig& config) {
    tile_size_ = config.tile_size;
    render_scale_ = config.render_scale;
    camera_.viewport_width = config.window_width;
    camera_.viewport_height = config.window_height;
}

void TileRenderer::get_reality_visuals(const Tile& tile, Reality current_reality, 
                                     int& out_texture_id, SDL_FColor& out_color) const {
    if (current_reality == Reality::A) {
        out_texture_id = (tile.reality_a_texture_id > 0) ? tile.reality_a_texture_id : tile.texture_id;
        out_color = (tile.reality_a_texture_id > 0) ? tile.reality_a_color : tile.color;
    } else {
        out_texture_id = (tile.reality_b_texture_id > 0) ? tile.reality_b_texture_id : tile.texture_id;
        out_color = (tile.reality_b_texture_id > 0) ? tile.reality_b_color : tile.color;
    }
}

void TileRenderer::render_reality_indicator(SDL_Renderer* renderer, Reality current_reality) {
    float indicator_x = camera_.viewport_width - reality_indicator_size_ - 20.0f;
    float indicator_y = 20.0f;
    
    const SDL_FColor& indicator_color = (current_reality == Reality::A) ? 
        reality_a_indicator_color_ : reality_b_indicator_color_;
    
    SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.7f);
    SDL_FRect background_rect = {
        indicator_x - 5.0f,
        indicator_y - 5.0f,
        reality_indicator_size_ + 10.0f,
        reality_indicator_size_ + 10.0f
    };
    SDL_RenderFillRect(renderer, &background_rect);
    
    SDL_SetRenderDrawColorFloat(renderer, indicator_color.r, indicator_color.g, 
                               indicator_color.b, indicator_color.a);
    SDL_FRect indicator_rect = {
        indicator_x,
        indicator_y,
        reality_indicator_size_,
        reality_indicator_size_
    };
    SDL_RenderFillRect(renderer, &indicator_rect);
    
    SDL_SetRenderDrawColorFloat(renderer, 1.0f, 1.0f, 1.0f, 1.0f);
    if (current_reality == Reality::A) {
        float center_x = indicator_x + reality_indicator_size_ * 0.5f;
        float center_y = indicator_y + reality_indicator_size_ * 0.5f;
        float line_size = reality_indicator_size_ * 0.3f;
        
        SDL_RenderLine(renderer, center_x, center_y + line_size, center_x - line_size, center_y - line_size);
        SDL_RenderLine(renderer, center_x, center_y + line_size, center_x + line_size, center_y - line_size);
        SDL_RenderLine(renderer, center_x - line_size * 0.5f, center_y, center_x + line_size * 0.5f, center_y);
    } else {
        float center_x = indicator_x + reality_indicator_size_ * 0.5f;
        float center_y = indicator_y + reality_indicator_size_ * 0.5f;
        float line_size = reality_indicator_size_ * 0.3f;
        
        SDL_FRect b_rect1 = {center_x - line_size, center_y - line_size, line_size * 0.5f, line_size * 0.8f};
        SDL_FRect b_rect2 = {center_x - line_size, center_y, line_size * 0.5f, line_size};
        SDL_RenderFillRect(renderer, &b_rect1);
        SDL_RenderFillRect(renderer, &b_rect2);
    }
}

void TileRenderer::set_reality_indicator_colors(const SDL_FColor& reality_a_color, const SDL_FColor& reality_b_color) {
    reality_a_indicator_color_ = reality_a_color;
    reality_b_indicator_color_ = reality_b_color;
}