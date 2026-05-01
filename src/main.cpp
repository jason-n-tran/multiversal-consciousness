#include "engine/GameEngine.h"
#include "engine/LevelLoader.h"
#include "engine/VerificationSystem.h"
#include "engine/PhysicsSystem.h"
#include "engine/QuantumLoadoutSystem.h"
#include "engine/HUDSystem.h"
#include "engine/PossessionSystem.h"
#include "engine/QuantumSystem.h"
#include "engine/RealityManager.h"
#include "engine/RealitySystem.h"
#include "engine/TileRenderer.h"
#include "engine/AgentRenderer.h"
#include "engine/QuantumNodeRenderer.h"
#include "engine/MovementSystem.h"
#include <iostream>
#include <memory>

int main() {
    try {
        auto game_engine = std::make_unique<GameEngine>();
        
        EngineConfig config;
        config.window_title = "Verification Demo - Phase 2 Systems";
        config.window_width = 1280;
        config.window_height = 720;
        config.vsync = true;
        
        if (!game_engine->initialize(config)) {
            std::cerr << "Failed to initialize game engine" << std::endl;
            return -1;
        }
        
        std::cout << "=== Quantum Bifurcation Verification Demo ===" << std::endl;
        std::cout << "Demonstrating Phase 2 gameplay systems:" << std::endl;
        std::cout << "- Physics simulation with gravity and collision" << std::endl;
        std::cout << "- Quantum loadout system with reality switching" << std::endl;
        std::cout << "- Interactive obstacles requiring specific abilities" << std::endl;
        std::cout << "- User interface with agent and ability display" << std::endl;
        std::cout << std::endl;
        std::cout.flush();
        
        auto& entity_manager = game_engine->get_entity_manager();
        auto& component_registry = game_engine->get_component_registry();
        
        auto reality_manager = std::make_unique<RealityManager>();
        auto quantum_system = std::make_unique<QuantumSystem>(std::move(reality_manager));
        auto possession_system = std::make_unique<PossessionSystem>();
        auto movement_system = std::make_unique<MovementSystem>();
        auto tile_renderer = std::make_unique<TileRenderer>(config);
        auto agent_renderer = std::make_unique<AgentRenderer>();
        auto quantum_node_renderer = std::make_unique<QuantumNodeRenderer>();
        
        auto physics_system = std::make_unique<PhysicsSystem>();
        auto quantum_loadout_system = std::make_unique<QuantumLoadoutSystem>();
        auto hud_system = std::make_unique<HUDSystem>();
        auto verification_system = std::make_unique<VerificationSystem>(&entity_manager, &component_registry);
        
        auto reality_system = std::make_unique<RealitySystem>();
        
        auto* quantum_sys_ptr = game_engine->register_system(std::move(quantum_system));
        auto* possession_sys_ptr = game_engine->register_system(std::move(possession_system));
        auto* movement_sys_ptr = game_engine->register_system(std::move(movement_system));
        auto* tile_renderer_ptr = game_engine->register_system(std::move(tile_renderer));
        auto* agent_renderer_ptr = game_engine->register_system(std::move(agent_renderer));
        auto* quantum_node_renderer_ptr = game_engine->register_system(std::move(quantum_node_renderer));
        auto* reality_sys_ptr = game_engine->register_system(std::move(reality_system));
        
        auto* physics_sys_ptr = game_engine->register_system(std::move(physics_system));
        auto* quantum_loadout_sys_ptr = game_engine->register_system(std::move(quantum_loadout_system));
        auto* hud_sys_ptr = game_engine->register_system(std::move(hud_system));
        game_engine->register_system(std::move(verification_system));
        
        possession_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        movement_sys_ptr->set_input_manager(&game_engine->get_input_manager());
        movement_sys_ptr->set_possession_system(possession_sys_ptr);
        movement_sys_ptr->set_loadout_system(quantum_loadout_sys_ptr);
        possession_sys_ptr->set_agent_renderer(agent_renderer_ptr);
        
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
        
        auto tile_map = std::make_unique<TileMap>();
        tile_map->initialize(25, 20);
        
        tile_renderer_ptr->create_solid_texture(1, {0.3f, 0.3f, 0.3f, 1.0f}, 32, 32, game_engine->get_renderer());
        tile_renderer_ptr->create_solid_texture(2, {0.1f, 0.1f, 0.1f, 1.0f}, 32, 32, game_engine->get_renderer()); 
        tile_renderer_ptr->create_solid_texture(3, {0.2f, 0.8f, 0.2f, 1.0f}, 32, 32, game_engine->get_renderer());
        tile_renderer_ptr->create_solid_texture(4, {0.8f, 0.2f, 0.8f, 1.0f}, 32, 32, game_engine->get_renderer()); 
        
        for (int y = 0; y < tile_map->height; ++y) {
            for (int x = 0; x < tile_map->width; ++x) {
                Tile tile;
                tile.texture_id = 2; 
                tile_map->set_tile(x, y, tile);
            }
        }
        
        tile_renderer_ptr->set_tile_map(std::move(tile_map));
        
        tile_renderer_ptr->set_show_grid(false);
        
        std::cout << "Game engine and systems initialized successfully" << std::endl;
        
        LevelLoader level_loader(&entity_manager, &component_registry);
        auto level_data = level_loader.load_level_from_file("levels/verification_scenario.level");
        
        if (level_data.name.empty()) {
            std::cerr << "Failed to load verification scenario level" << std::endl;
            return -1;
        }
        
        std::cout << "Loaded level: " << level_data.name << std::endl;
        std::cout << "Description: " << level_data.description << std::endl;
        std::cout << std::endl;
        std::cout.flush(); 
        
        auto created_entities = level_loader.instantiate_level(level_data);
        std::cout << "Created " << created_entities.size() << " entities from level data" << std::endl;
        
        for (EntityID entity_id : created_entities) {
            auto* transform = component_registry.get_component<Transform>(entity_id);
            auto* agent = component_registry.get_component<Agent>(entity_id);
            auto* wall = component_registry.get_component<Wall>(entity_id);
            auto* quantum_node = component_registry.get_component<QuantumNode>(entity_id);
            auto* trigger = component_registry.get_component<Trigger>(entity_id);
            
            std::cout << "Entity " << entity_id;
            if (transform) std::cout << " at (" << transform->x << ", " << transform->y << ")";
            if (agent) std::cout << " [AGENT " << static_cast<int>(agent->agent_number) << "]";
            if (wall) std::cout << " [WALL " << wall->width << "x" << wall->height << " solid:" << wall->is_solid << "]";
            if (quantum_node) std::cout << " [QUANTUM_NODE]";
            if (trigger) std::cout << " [TRIGGER " << trigger->trigger_type << "]";
            std::cout << std::endl;
        }
        std::cout.flush(); 
        
        const auto* agent_container = component_registry.get_all_components<Agent>();
        if (agent_container) {
            const auto& agent_entities = agent_container->get_entities();
            std::cout << "Found " << agent_entities.size() << " agent entities" << std::endl;
            for (EntityID agent_entity : agent_entities) {
                auto* transform = component_registry.get_component<Transform>(agent_entity);
                if (transform) {
                    std::cout << "Agent entity " << agent_entity << " at (" << transform->x << ", " << transform->y << ")" << std::endl;
                }
                
                PhysicsComponent physics;
                physics.apply_gravity = true;
                physics.mass = 1.0f;
                physics.friction = 0.8f;
                component_registry.add_component(agent_entity, std::move(physics));
                
                BoundingBoxComponent bbox;
                bbox.width = 32.0f;
                bbox.height = 32.0f;
                bbox.is_solid = false; 
                component_registry.add_component(agent_entity, std::move(bbox));
                
                LoadoutComponent loadout;
                loadout.reality_abilities["A"] = AbilityType::None;
                loadout.reality_abilities["B"] = AbilityType::None; 
                loadout.current_ability = AbilityType::None; 
                component_registry.add_component(agent_entity, std::move(loadout));
                
                auto* agent = component_registry.get_component<Agent>(agent_entity);
                if (agent && agent->agent_number == 1) {
                    agent->is_possessed = true;
                    std::cout << "Agent " << static_cast<int>(agent->agent_number) << " is now possessed" << std::endl;
                }
            }
        }
        
        const auto* wall_container = component_registry.get_all_components<Wall>();
        if (wall_container) {
            const auto& wall_entities = wall_container->get_entities();
            std::cout << "Found " << wall_entities.size() << " wall entities" << std::endl;
            for (EntityID wall_entity : wall_entities) {
                auto* wall = component_registry.get_component<Wall>(wall_entity);
                auto* transform = component_registry.get_component<Transform>(wall_entity);
                if (wall && transform) {
                    BoundingBoxComponent bbox;
                    bbox.width = wall->width;
                    bbox.height = wall->height;
                    bbox.is_solid = wall->is_solid;
                    component_registry.add_component(wall_entity, std::move(bbox));
                    
                    std::cout << "Wall entity " << wall_entity << " at (" << transform->x << ", " << transform->y 
                              << ") size: " << wall->width << "x" << wall->height 
                              << " solid: " << wall->is_solid << std::endl;
                    
                    float left = transform->x - (wall->width * 0.5f);
                    float top = transform->y - (wall->height * 0.5f);
                    
                    int tile_x = static_cast<int>(std::round(left / 32.0f));
                    int tile_y = static_cast<int>(std::round(top / 32.0f));
                    int tile_width = static_cast<int>(std::ceil(wall->width / 32.0f));
                    int tile_height = static_cast<int>(std::ceil(wall->height / 32.0f));
                    
                    std::cout << "Wall at physics (" << transform->x << ", " << transform->y 
                              << ") -> tiles (" << tile_x << ", " << tile_y 
                              << ") size " << tile_width << "x" << tile_height << std::endl;
                    
                    for (int y = tile_y; y < tile_y + tile_height && y < 20; ++y) {
                        for (int x = tile_x; x < tile_x + tile_width && x < 25; ++x) {
                            if (x >= 0 && y >= 0) {
                                Tile tile;
                                tile.texture_id = 1; // Dark gray wall texture
                                tile_renderer_ptr->get_tile_map()->set_tile(x, y, tile);
                            }
                        }
                    }
                }
            }
        }
        
        const auto* trigger_container = component_registry.get_all_components<Trigger>();
        if (trigger_container) {
            const auto& trigger_entities = trigger_container->get_entities();
            for (EntityID trigger_entity : trigger_entities) {
                auto* trigger = component_registry.get_component<Trigger>(trigger_entity);
                auto* transform = component_registry.get_component<Transform>(trigger_entity);
                if (trigger && transform && trigger->trigger_type == "success_zone") {
                    BoundingBoxComponent bbox;
                    bbox.width = trigger->width;
                    bbox.height = trigger->height;
                    bbox.is_solid = false; 
                    bbox.is_trigger = true;
                    component_registry.add_component(trigger_entity, std::move(bbox));
                    
                    float left = transform->x - (trigger->width * 0.5f);
                    float top = transform->y - (trigger->height * 0.5f);
                    
                    int tile_x = static_cast<int>(std::round(left / 32.0f));
                    int tile_y = static_cast<int>(std::round(top / 32.0f));
                    int tile_width = static_cast<int>(std::ceil(trigger->width / 32.0f));
                    int tile_height = static_cast<int>(std::ceil(trigger->height / 32.0f));
                    
                    std::cout << "Success zone at physics (" << transform->x << ", " << transform->y 
                              << ") -> tiles (" << tile_x << ", " << tile_y 
                              << ") size " << tile_width << "x" << tile_height << std::endl;
                    
                    for (int y = tile_y; y < tile_y + tile_height && y < 20; ++y) {
                        for (int x = tile_x; x < tile_x + tile_width && x < 25; ++x) {
                            if (x >= 0 && y >= 0) {
                                Tile tile;
                                tile.texture_id = 3; // Green success zone texture
                                tile_renderer_ptr->get_tile_map()->set_tile(x, y, tile);
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << std::endl;
        std::cout << "=== Verification Scenario Instructions ===" << std::endl;
        std::cout << "1. Use A/D keys to move the agent left/right" << std::endl;
        std::cout << "2. Use W/Space to jump (basic jump only initially)" << std::endl;
        std::cout << "3. Approach the quantum node (purple square) and press E to interact" << std::endl;
        std::cout << "4. This will switch to Reality B and grant DoubleJump ability" << std::endl;
        std::cout << "5. Use DoubleJump (W/Space twice) to traverse the high wall" << std::endl;
        std::cout << "6. Reach the green success zone to complete the verification" << std::endl;
        std::cout << "7. Press R to switch realities manually, ESC to quit" << std::endl;
        std::cout << std::endl;
        std::cout << "=== Controls ===" << std::endl;
        std::cout << "A/D: Move agent left/right" << std::endl;
        std::cout << "W/Space: Jump (DoubleJump available in Reality B after quantum interaction)" << std::endl;
        std::cout << "E: Interact with quantum nodes" << std::endl;
        std::cout << "R: Switch reality (Reality A <-> Reality B)" << std::endl;
        std::cout << "ESC: Exit demo" << std::endl;
        std::cout << std::endl;
        
        game_engine->run();
        
        std::cout << "Verification demo completed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in verification demo: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception in verification demo" << std::endl;
        return -1;
    }
    
    return 0;
}