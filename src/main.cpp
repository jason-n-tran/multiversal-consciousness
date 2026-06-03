#include "engine/GameEngine.h"
#include "engine/LevelLoader.h"
#include "engine/VerificationSystem.h"
#include "engine/PhysicsSystem.h"
#include "engine/QuantumLoadoutSystem.h"
#include "engine/HUDSystem.h"
#include "engine/PossessionSystem.h"
#include "engine/QuantumSystem.h"
#include "engine/RealitySystem.h"
#include "engine/TileRenderer.h"
#include "engine/AgentRenderer.h"
#include "engine/QuantumNodeRenderer.h"
#include "engine/MovementSystem.h"
#include "engine/InteractiveObstacleSystem.h"
#include "engine/MenuSystem.h"
#include "engine/BackgroundSystem.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

void configure_level_visuals(TileRenderer* tile_renderer_ptr, SDL_Renderer* renderer);
void process_level_entities(ComponentRegistry& component_registry, 
                           TileRenderer* tile_renderer_ptr, 
                           InteractiveObstacleSystem* interactive_obstacle_sys_ptr);


class GameFlowSystem : public ISystem {
public:
    using LoadLevelFunc = std::function<void(const std::string&, int)>;

    GameFlowSystem(std::vector<std::string> levels, LoadLevelFunc load_level_cb)
        : levels_(std::move(levels)), load_level_cb_(std::move(load_level_cb)) {}

    void set_verification_system(VerificationSystem* vs) { verification_system_ = vs; }
    void set_menu_system(MenuSystem* ms) { menu_system_ = ms; }
    void set_engine(GameEngine* engine) { engine_ = engine; }

    void start_game() {
        current_index_ = 0;
        game_completed_ = false;
        transition_timer_ = 0.0f;
        load_level_cb_(levels_[current_index_], current_index_);
        if (menu_system_) {
            menu_system_->set_menu_mode(MenuMode::Pause);
        }
    }

    void reset_current_level() {
        if (current_index_ >= 0 && current_index_ < (int)levels_.size()) {
            load_level_cb_(levels_[current_index_], current_index_);
        }
    }

    bool run_while_paused() const override { return true; }

    void update(float delta_time) override {
        if (game_completed_ || current_index_ < 0) return;

        if (verification_system_ && verification_system_->is_scenario_completed()) {
            if (transition_timer_ == 0.0f) {
                std::cout << "Level " << (current_index_ + 1) << " completed! Transitioning in 3s..." << std::endl;
            }
            
            transition_timer_ += delta_time;
            
            if (transition_timer_ >= 3.0f) {
                transition_timer_ = 0.0f;
                current_index_++;
                
                if (current_index_ < (int)levels_.size()) {
                    load_level_cb_(levels_[current_index_], current_index_);
                } else {
                    game_completed_ = true;
                    std::cout << "All levels completed! Showing Congratulations." << std::endl;
                    if (menu_system_ && engine_) {
                        menu_system_->set_menu_mode(MenuMode::Congratulations);
                        engine_->set_paused(true);
                    }
                }
            }
        }
    }

private:
    std::vector<std::string> levels_;
    LoadLevelFunc load_level_cb_;
    VerificationSystem* verification_system_ = nullptr;
    MenuSystem* menu_system_ = nullptr;
    GameEngine* engine_ = nullptr;
    
    int current_index_ = -1;
    float transition_timer_ = 0.0f;
    bool game_completed_ = false;
};

int main() {
    try {
        auto game_engine = std::make_unique<GameEngine>();
        
        EngineConfig config;
        config.window_title = "Multiversal Consciousness - Main Runner";
        config.window_width = 1280;
        config.window_height = 720;
        config.vsync = true;
        
        if (!game_engine->initialize(config)) {
            std::cerr << "Failed to initialize game engine" << std::endl;
            return -1;
        }
        
        auto& entity_manager = game_engine->get_entity_manager();
        auto& component_registry = game_engine->get_component_registry();
        
        auto reality_system = std::make_unique<RealitySystem>();
        auto quantum_system = std::make_unique<QuantumSystem>(&reality_system->get_reality_manager());
        auto possession_system = std::make_unique<PossessionSystem>();
        auto movement_system = std::make_unique<MovementSystem>();
        auto background_system = std::make_unique<BackgroundSystem>();
        
        auto tile_renderer = std::make_unique<TileRenderer>(config);
        auto agent_renderer = std::make_unique<AgentRenderer>();
        auto quantum_node_renderer = std::make_unique<QuantumNodeRenderer>();
        auto interactive_obstacle_system = std::make_unique<InteractiveObstacleSystem>();
        auto physics_system = std::make_unique<PhysicsSystem>();
        auto quantum_loadout_system = std::make_unique<QuantumLoadoutSystem>();
        auto hud_system = std::make_unique<HUDSystem>();
        auto verification_system = std::make_unique<VerificationSystem>(&entity_manager, &component_registry);
        auto menu_system = std::make_unique<MenuSystem>(game_engine.get());

        auto* background_sys_ptr = game_engine->register_system(std::move(background_system));
        
        auto* quantum_sys_ptr = game_engine->register_system(std::move(quantum_system));
        auto* possession_sys_ptr = game_engine->register_system(std::move(possession_system));
        auto* movement_sys_ptr = game_engine->register_system(std::move(movement_system));
        auto* tile_renderer_ptr = game_engine->register_system(std::move(tile_renderer));
        auto* agent_renderer_ptr = game_engine->register_system(std::move(agent_renderer));
        auto* quantum_node_renderer_ptr = game_engine->register_system(std::move(quantum_node_renderer));
        auto* reality_sys_ptr = game_engine->register_system(std::move(reality_system));
        auto* interactive_obstacle_sys_ptr = game_engine->register_system(std::move(interactive_obstacle_system));
        game_engine->register_system(std::move(physics_system));
        auto* quantum_loadout_sys_ptr = game_engine->register_system(std::move(quantum_loadout_system));
        auto* hud_sys_ptr = game_engine->register_system(std::move(hud_system));
        auto* verification_sys_ptr = game_engine->register_system(std::move(verification_system));
        auto* menu_sys_ptr = game_engine->register_system(std::move(menu_system));
        
        possession_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        movement_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        movement_sys_ptr->set_possession_system(possession_sys_ptr);
        movement_sys_ptr->set_loadout_system(quantum_loadout_sys_ptr);
        possession_sys_ptr->set_agent_renderer(agent_renderer_ptr);
        interactive_obstacle_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        interactive_obstacle_sys_ptr->set_possession_system(possession_sys_ptr);
        interactive_obstacle_sys_ptr->set_hud_system(hud_sys_ptr);
        quantum_loadout_sys_ptr->set_reality_system(reality_sys_ptr);
        agent_renderer_ptr->set_camera_controller(&possession_sys_ptr->get_camera_controller());
        quantum_node_renderer_ptr->set_camera_controller(&possession_sys_ptr->get_camera_controller());
        tile_renderer_ptr->set_camera_controller(&possession_sys_ptr->get_camera_controller());
        tile_renderer_ptr->set_reality_manager(&reality_sys_ptr->get_reality_manager());
        agent_renderer_ptr->set_reality_manager(&reality_sys_ptr->get_reality_manager());
        quantum_node_renderer_ptr->set_reality_manager(&reality_sys_ptr->get_reality_manager());
        reality_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        quantum_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        quantum_sys_ptr->set_possession_system(possession_sys_ptr);
        quantum_sys_ptr->set_loadout_system(quantum_loadout_sys_ptr);
        hud_sys_ptr->set_possession_system(possession_sys_ptr);
        hud_sys_ptr->set_loadout_system(quantum_loadout_sys_ptr);
        hud_sys_ptr->set_reality_manager(&reality_sys_ptr->get_reality_manager());
        
        // Connect BackgroundSystem
        background_sys_ptr->set_camera_controller(&possession_sys_ptr->get_camera_controller());
        
        HUDVisualConfig hud_config;
        hud_sys_ptr->set_visual_config(hud_config);
        agent_renderer_ptr->initialize_sprites(game_engine->get_renderer());
        quantum_loadout_sys_ptr->set_hud_system(hud_sys_ptr);
        reality_sys_ptr->set_hud_system(hud_sys_ptr);
        verification_sys_ptr->set_hud_system(hud_sys_ptr);
        verification_sys_ptr->set_reality_manager(&reality_sys_ptr->get_reality_manager());

        auto load_level = [&](const std::string& level_path, int level_index) {
            std::cout << "Loading Level: " << level_path << " (Index: " << level_index << ")" << std::endl;
            entity_manager.clear();
            component_registry.clear();
            possession_sys_ptr->reset();
            interactive_obstacle_sys_ptr->reset();
            reality_sys_ptr->reset();
            quantum_loadout_sys_ptr->reset();
            verification_sys_ptr->reset();
            
            background_sys_ptr->set_level_index(level_index);
            
            auto tile_map = std::make_unique<TileMap>();
            Tile empty_tile;
            empty_tile.visible = false;
            tile_map->initialize(100, 50, empty_tile); 
            
            configure_level_visuals(tile_renderer_ptr, game_engine->get_renderer());
            
            tile_renderer_ptr->set_tile_map(std::move(tile_map));
            tile_renderer_ptr->set_show_grid(false);

            LevelLoader level_loader(&entity_manager, &component_registry);
            auto level_data = level_loader.load_level_from_file(level_path);
            if (level_data.name.empty()) {
                std::cerr << "Failed to load level: " << level_path << std::endl;
                return;
            }

            verification_sys_ptr->set_conditions(level_data.completion_conditions);
            level_loader.instantiate_level(level_data);

            CameraBounds camera_bounds;
            float map_pixel_width = 100.0f * 32.0f; 
            float map_pixel_height = 50.0f * 32.0f; 
            
            auto* current_map = tile_renderer_ptr->get_tile_map();
            if (current_map) {
                map_pixel_width = static_cast<float>(current_map->width * 32);
                map_pixel_height = static_cast<float>(current_map->height * 32);
            }
            
            camera_bounds.min_x = -200.0f; 
            camera_bounds.max_x = map_pixel_width + 200.0f;
            camera_bounds.min_y = -200.0f; 
            camera_bounds.max_y = map_pixel_height + 200.0f;
            possession_sys_ptr->set_camera_bounds(camera_bounds);

            const auto* agent_container = component_registry.get_all_components<Agent>();
            if (agent_container) {
                for (EntityID agent_entity : agent_container->get_entities()) {
                    PhysicsComponent physics; physics.apply_gravity = true;
                    component_registry.add_component(agent_entity, std::move(physics));
                    BoundingBoxComponent bbox; bbox.width = 32.0f; bbox.height = 32.0f; bbox.is_solid = false;
                    component_registry.add_component(agent_entity, std::move(bbox));
                    LoadoutComponent loadout;
                    component_registry.add_component(agent_entity, std::move(loadout));
                }
            }

            process_level_entities(component_registry, tile_renderer_ptr, interactive_obstacle_sys_ptr);
            
            if (possession_sys_ptr->possess_agent(1)) {
                possession_sys_ptr->snap_camera_to_possessed();
            }

            hud_sys_ptr->set_verification_message("Press ESC for Menu", 3.0f);
        };

        std::vector<std::string> game_levels = {
            "levels/simple_example.level",
            "levels/verification_scenario.level",
            "levels/tutorial.level",
            "levels/movement.level",
            "levels/abilities_obstacles.level",
            "levels/beginner.level",
        };
        auto game_flow_system = std::make_unique<GameFlowSystem>(game_levels, load_level);
        game_flow_system->set_verification_system(verification_sys_ptr);
        game_flow_system->set_menu_system(menu_sys_ptr);
        game_flow_system->set_engine(game_engine.get());
        auto* flow_sys_ptr = game_engine->register_system(std::move(game_flow_system));

        menu_sys_ptr->set_start_callback([flow_sys_ptr]() { flow_sys_ptr->start_game(); });
        menu_sys_ptr->set_reset_callback([flow_sys_ptr]() { flow_sys_ptr->reset_current_level(); });
        
        menu_sys_ptr->set_menu_mode(MenuMode::Start);
        game_engine->set_paused(true);

        std::cout << "Starting Multiversal Consciousness Runner..." << std::endl;
        game_engine->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

void configure_level_visuals(TileRenderer* tile_renderer_ptr, SDL_Renderer* renderer) {
    tile_renderer_ptr->create_solid_texture(1, {0.4f, 0.4f, 0.4f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(2, {0.1f, 0.1f, 0.1f, 1.0f}, 32, 32, renderer);
    tile_renderer_ptr->create_solid_texture(3, {0.2f, 0.8f, 0.2f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(4, {0.8f, 0.2f, 0.8f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(5, {0.6f, 0.3f, 0.1f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(6, {0.8f, 0.6f, 0.2f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(7, {0.2f, 0.6f, 1.0f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(8, {1.0f, 0.2f, 0.2f, 1.0f}, 32, 32, renderer); 
    tile_renderer_ptr->create_solid_texture(9, {0.0f, 0.0f, 0.0f, 0.0f}, 32, 32, renderer); 
    tile_renderer_ptr->load_texture(10, "assets/ground.png", renderer);
    tile_renderer_ptr->create_solid_texture(11, {0.3f, 0.9f, 0.9f, 0.6f}, 32, 32, renderer); 
}

void process_level_entities(ComponentRegistry& component_registry, 
                           TileRenderer* tile_renderer_ptr, 
                           InteractiveObstacleSystem* interactive_obstacle_sys_ptr) {
    
    auto* tile_map = tile_renderer_ptr->get_tile_map();
    if (!tile_map) return;

    const auto* wall_container = component_registry.get_all_components<Wall>();
    if (wall_container) {
        for (EntityID entity : wall_container->get_entities()) {
            auto* wall = component_registry.get_component<Wall>(entity);
            auto* trans = component_registry.get_component<Transform>(entity);
            if (wall && trans) {
                int tx = static_cast<int>((trans->x - wall->width/2) / 32);
                int ty = static_cast<int>((trans->y - wall->height/2) / 32);
                int tw = static_cast<int>(wall->width/32);
                int th = static_cast<int>(wall->height/32);
                int tex = 1;
                if (wall->wall_type == "tree") {
                    tex = 5;
                    InteractableComponent interact;
                    interact.type = InteractionType::Tree;
                    interact.required_ability = AbilityType::Axe;
                    interact.interaction_radius = 48.0f;
                    interact.interaction_text = "Press E to chop tree (Axe)";
                    component_registry.add_component(entity, std::move(interact));
                    interactive_obstacle_sys_ptr->register_obstacle(entity);
                } else if (wall->wall_type == "ground") tex = 10;
                else if (wall->wall_type == "phaseable") tex = 11;
                
                for(int y=ty; y<ty+th; ++y) for(int x=tx; x<tx+tw; ++x) {
                    if(x>=0 && x<tile_map->width && y>=0 && y<tile_map->height) {
                        Tile t; 
                        t.texture_id = tex; 
                        t.layer = (tex == 1 || tex == 2 || tex == 3 || tex == 4 || tex == 9 || tex == 10 || tex == 11 ? 0 : 1);
                        tile_map->set_tile(x, y, t);
                    }
                }
            }
        }
    }

    const auto* door_container = component_registry.get_all_components<Door>();
    if (door_container) {
        for (EntityID entity : door_container->get_entities()) {
            auto* door = component_registry.get_component<Door>(entity);
            auto* trans = component_registry.get_component<Transform>(entity);
            if (door && trans) {
                int tx = static_cast<int>(trans->x / 32);
                int ty = static_cast<int>(trans->y / 32);
                if(tx>=0 && tx<tile_map->width && ty>=0 && ty<tile_map->height) {
                    Tile t; t.texture_id = 6; t.layer = 1; tile_map->set_tile(tx, ty, t);
                }
                BoundingBoxComponent bbox; bbox.width = 32.0f; bbox.height = 32.0f; bbox.is_solid = door->is_locked;
                component_registry.add_component(entity, std::move(bbox));
                InteractableComponent interact;
                interact.type = InteractionType::Door;
                interact.required_ability = AbilityType::Keycard;
                interact.interaction_radius = 48.0f;
                interact.interaction_text = "Press E to unlock (Keycard)";
                component_registry.add_component(entity, std::move(interact));
                interactive_obstacle_sys_ptr->register_obstacle(entity);
            }
        }
    }

    const auto* trigger_container = component_registry.get_all_components<Trigger>();
    if (trigger_container) {
        for (EntityID entity : trigger_container->get_entities()) {
            auto* trig = component_registry.get_component<Trigger>(entity);
            auto* trans = component_registry.get_component<Transform>(entity);
            if (trig && trans && trig->trigger_type == "success_zone") {
                int tx = static_cast<int>((trans->x - trig->width/2) / 32);
                int ty = static_cast<int>((trans->y - trig->height/2) / 32);
                int tw = static_cast<int>(trig->width/32);
                int th = static_cast<int>(trig->height/32);
                for(int y=ty; y<ty+th; ++y) for(int x=tx; x<tx+tw; ++x) {
                    if(x>=0 && x<tile_map->width && y>=0 && y<tile_map->height) {
                        Tile t; t.texture_id = 3; t.layer = 0; tile_map->set_tile(x, y, t);
                    }
                }
            }
        }
    }
    
    const auto* water_container = component_registry.get_all_components<WaterLevel>();
    if (water_container) {
        for (EntityID entity : water_container->get_entities()) {
            auto* trans = component_registry.get_component<Transform>(entity);
            auto* water = component_registry.get_component<WaterLevel>(entity);
            if (trans && water) {
                int tx = static_cast<int>(trans->x / 32);
                int ty = static_cast<int>(trans->y / 32);
                if(tx>=0 && tx<tile_map->width && ty>=0 && ty<tile_map->height) {
                    Tile t; t.texture_id = 7; t.layer = 1; tile_map->set_tile(tx, ty, t);
                }
                BoundingBoxComponent bbox;
                bbox.width = 32.0f;
                bbox.height = 32.0f;
                bbox.is_solid = false;
                bbox.is_trigger = true;
                component_registry.add_component(entity, std::move(bbox));
            }
        }
    }

    const auto* switch_container = component_registry.get_all_components<EnvironmentalSwitch>();
    if (switch_container) {
        for (EntityID entity : switch_container->get_entities()) {
            auto* trans = component_registry.get_component<Transform>(entity);
            if (trans) {
                int tx = static_cast<int>(trans->x / 32);
                int ty = static_cast<int>(trans->y / 32);
                if(tx>=0 && tx<tile_map->width && ty>=0 && ty<tile_map->height) {
                    Tile t; t.texture_id = 8; t.layer = 1; tile_map->set_tile(tx, ty, t);
                }
                InteractableComponent interact;
                interact.type = InteractionType::Switch;
                interact.interaction_radius = 48.0f;
                interact.interaction_text = "Press E to activate";
                component_registry.add_component(entity, std::move(interact));
                interactive_obstacle_sys_ptr->register_obstacle(entity);
            }
        }
    }
}