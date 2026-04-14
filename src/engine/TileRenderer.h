#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "System.h"
#include "SDLDeleter.h"
#include "ConfigLoader.h"

struct Tile {
    int texture_id = 0;
    SDL_FRect source_rect{0.0f, 0.0f, 32.0f, 32.0f};
    SDL_FColor color{1.0f, 1.0f, 1.0f, 1.0f};
    int layer = 0;
    bool visible = true;
};

struct TileMap {
    std::vector<std::vector<Tile>> tiles;
    int width = 0;
    int height = 0;
    int tile_size = 32;
    
    Tile* get_tile(int x, int y);
    
    const Tile* get_tile(int x, int y) const;
    
    bool set_tile(int x, int y, const Tile& tile);
    
    void initialize(int w, int h, const Tile& default_tile = {});
    
    void clear();
};

struct Camera {
    float x = 0.0f;
    float y = 0.0f;
    float zoom = 1.0f;
    int viewport_width = 1280;
    int viewport_height = 720;
    
    void world_to_screen(float world_x, float world_y, int& screen_x, int& screen_y) const;
    
    void screen_to_world(int screen_x, int screen_y, float& world_x, float& world_y) const;
    
    bool is_visible(float world_x, float world_y, float width, float height) const;
};

class TileRenderer : public IRenderSystem {
private:
    std::unordered_map<int, TexturePtr> textures_;
    std::unique_ptr<TileMap> tile_map_;
    Camera camera_;
    
    int tile_size_;
    float render_scale_;
    bool show_grid_;
    SDL_FColor grid_color_;
    
    mutable int visible_tiles_rendered_;
    
public:
    explicit TileRenderer(const EngineConfig& config);
    
    ~TileRenderer() override = default;
    
    TileRenderer(const TileRenderer&) = delete;
    TileRenderer& operator=(const TileRenderer&) = delete;
    
    TileRenderer(TileRenderer&&) noexcept = default;
    TileRenderer& operator=(TileRenderer&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void render(SDL_Renderer* renderer) override;
    
    void shutdown() override;
    
    bool load_texture(int texture_id, const std::string& filename, SDL_Renderer* renderer);
    
    bool create_solid_texture(int texture_id, SDL_FColor color, int width, int height, SDL_Renderer* renderer);
    
    TileMap* get_tile_map() { return tile_map_.get(); }
    
    const TileMap* get_tile_map() const { return tile_map_.get(); }
    
    void set_tile_map(std::unique_ptr<TileMap> tile_map);
    
    Camera& get_camera() { return camera_; }
    
    const Camera& get_camera() const { return camera_; }
    
    void set_camera_position(float x, float y);
    
    void set_camera_zoom(float zoom);
    
    void set_show_grid(bool show_grid) { show_grid_ = show_grid; }
    
    int get_visible_tiles_rendered() const { return visible_tiles_rendered_; }
    
    void update_config(const EngineConfig& config);
};