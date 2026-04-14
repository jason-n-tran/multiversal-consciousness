#include "TileRenderer.h"
#include <iostream>
#include <algorithm>

Tile* TileMap::get_tile(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return nullptr;
    }
    return &tiles[y][x];
}

const Tile* TileMap::get_tile(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return nullptr;
    }
    return &tiles[y][x];
}

bool TileMap::set_tile(int x, int y, const Tile& tile) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }
    tiles[y][x] = tile;
    return true;
}

void TileMap::initialize(int w, int h, const Tile& default_tile) {
    width = w;
    height = h;
    tiles.clear();
    tiles.resize(height);
    for (int y = 0; y < height; ++y) {
        tiles[y].resize(width, default_tile);
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
    (void)delta_time;
}

void TileRenderer::render(SDL_Renderer* renderer) {
    if (!tile_map_ || !renderer) {
        return;
    }
    
    visible_tiles_rendered_ = 0;
    
    float world_left, world_top, world_right, world_bottom;
    camera_.screen_to_world(0, 0, world_left, world_top);
    camera_.screen_to_world(camera_.viewport_width, camera_.viewport_height, world_right, world_bottom);
    
    int tile_left = std::max(0, static_cast<int>(world_left / tile_size_) - 1);
    int tile_top = std::max(0, static_cast<int>(world_top / tile_size_) - 1);
    int tile_right = std::min(tile_map_->width - 1, static_cast<int>(world_right / tile_size_) + 1);
    int tile_bottom = std::min(tile_map_->height - 1, static_cast<int>(world_bottom / tile_size_) + 1);
    
    for (int layer = 0; layer <= 10; ++layer) {
        for (int y = tile_top; y <= tile_bottom; ++y) {
            for (int x = tile_left; x <= tile_right; ++x) {
                const Tile* tile = tile_map_->get_tile(x, y);
                if (!tile || !tile->visible || tile->layer != layer) {
                    continue;
                }
                
                float world_x = x * tile_size_;
                float world_y = y * tile_size_;
                
                int screen_x, screen_y;
                camera_.world_to_screen(world_x, world_y, screen_x, screen_y);
                
                int scaled_size = static_cast<int>(tile_size_ * camera_.zoom * render_scale_);
                
                SDL_FRect dest_rect{
                    static_cast<float>(screen_x),
                    static_cast<float>(screen_y),
                    static_cast<float>(scaled_size),
                    static_cast<float>(scaled_size)
                };
                
                if (tile->texture_id > 0) {
                    auto texture_it = textures_.find(tile->texture_id);
                    if (texture_it != textures_.end()) {
                        SDL_SetTextureColorMod(texture_it->second.get(), 
                                             static_cast<Uint8>(tile->color.r * 255),
                                             static_cast<Uint8>(tile->color.g * 255),
                                             static_cast<Uint8>(tile->color.b * 255));
                        SDL_SetTextureAlphaMod(texture_it->second.get(), 
                                             static_cast<Uint8>(tile->color.a * 255));
                        
                        SDL_RenderTexture(renderer, texture_it->second.get(), 
                                        &tile->source_rect, &dest_rect);
                    }
                } else {
                    SDL_SetRenderDrawColorFloat(renderer, tile->color.r, tile->color.g, 
                                              tile->color.b, tile->color.a);
                    SDL_RenderFillRect(renderer, &dest_rect);
                }
                
                visible_tiles_rendered_++;
            }
        }
    }
    
    if (show_grid_) {
        SDL_SetRenderDrawColorFloat(renderer, grid_color_.r, grid_color_.g, 
                                  grid_color_.b, grid_color_.a);
        
        for (int x = tile_left; x <= tile_right + 1; ++x) {
            float world_x = x * tile_size_;
            int screen_x, screen_y_top, screen_y_bottom;
            camera_.world_to_screen(world_x, tile_top * tile_size_, screen_x, screen_y_top);
            camera_.world_to_screen(world_x, (tile_bottom + 1) * tile_size_, screen_x, screen_y_bottom);
            SDL_RenderLine(renderer, screen_x, screen_y_top, screen_x, screen_y_bottom);
        }
        
        for (int y = tile_top; y <= tile_bottom + 1; ++y) {
            float world_y = y * tile_size_;
            int screen_x_left, screen_x_right, screen_y;
            camera_.world_to_screen(tile_left * tile_size_, world_y, screen_x_left, screen_y);
            camera_.world_to_screen((tile_right + 1) * tile_size_, world_y, screen_x_right, screen_y);
            SDL_RenderLine(renderer, screen_x_left, screen_y, screen_x_right, screen_y);
        }
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
    
    SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << filename << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
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