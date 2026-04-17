#include "engine/GameEngine.h"
#include "engine/PossessionSystem.h"
#include "engine/AgentRenderer.h"
#include "engine/Components.h"
#include <iostream>

int main() {
    GameEngine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize game engine" << std::endl;
        return -1;
    }
    
    auto& entity_manager = engine.get_entity_manager();
    auto& component_registry = engine.get_component_registry();
    auto& system_manager = engine.get_system_manager();
    
    auto possession_system = system_manager.register_system(std::make_unique<PossessionSystem>());
    auto agent_renderer = system_manager.register_system(std::make_unique<AgentRenderer>());
    
    system_manager.initialize();
    
    possession_system->set_agent_renderer(agent_renderer);
    
    std::cout << "Creating test agents..." << std::endl;
    
    EntityID agent1 = entity_manager.create_entity();
    Agent breacher{1, false, 120.0f};
    Transform transform1{100.0f, 200.0f, 0.0f, 1.0f, 1.0f};
    Renderable renderable1{"agent_texture", {0, 0, 32, 32}, 1.0f, 0.0f, 0.0f, 1.0f, 1};
    
    component_registry.add_component<Agent>(agent1, breacher);
    component_registry.add_component<Transform>(agent1, transform1);
    component_registry.add_component<Renderable>(agent1, renderable1);
    
    EntityID agent2 = entity_manager.create_entity();
    Agent runner{2, false, 200.0f};
    Transform transform2{300.0f, 400.0f, 0.0f, 1.0f, 1.0f};
    Renderable renderable2{"agent_texture", {0, 0, 32, 32}, 0.0f, 1.0f, 0.0f, 1.0f, 1};
    
    component_registry.add_component<Agent>(agent2, runner);
    component_registry.add_component<Transform>(agent2, transform2);
    component_registry.add_component<Renderable>(agent2, renderable2);
    
    EntityID agent3 = entity_manager.create_entity();
    Agent engineer{3, false, 80.0f};
    Transform transform3{500.0f, 600.0f, 0.0f, 1.0f, 1.0f};
    Renderable renderable3{"agent_texture", {0, 0, 32, 32}, 0.0f, 0.0f, 1.0f, 1.0f, 1};
    
    component_registry.add_component<Agent>(agent3, engineer);
    component_registry.add_component<Transform>(agent3, transform3);
    component_registry.add_component<Renderable>(agent3, renderable3);
    
    std::cout << "Agents created successfully!" << std::endl;
    
    system_manager.update(0.016f);
    
    std::cout << "\n=== Possession System Demo ===" << std::endl;
    
    const auto& mappings = possession_system->get_agent_mappings();
    std::cout << "Available agents:" << std::endl;
    for (const auto& [number, entity] : mappings) {
        std::cout << "  Agent " << static_cast<int>(number) << " -> Entity ID " << entity << std::endl;
    }
    
    std::cout << "\nTesting possession..." << std::endl;
    
    if (possession_system->possess_agent(1)) {
        std::cout << "Successfully possessed Agent 1 (Breacher)" << std::endl;
        auto possessed = possession_system->get_possessed_entity();
        if (possessed.has_value()) {
            std::cout << "Currently possessed entity: " << possessed.value() << std::endl;
        }
    }
    
    const auto& camera = possession_system->get_camera_controller();
    if (camera.get_target_entity().has_value()) {
        std::cout << "Camera is following Entity ID: " << camera.get_target_entity().value() << std::endl;
        std::cout << "Camera position: (" << camera.get_x() << ", " << camera.get_y() << ")" << std::endl;
    }
    
    std::cout << "\nSwitching to Agent 2 (Runner)..." << std::endl;
    if (possession_system->possess_agent(2)) {
        std::cout << "Successfully switched to Agent 2" << std::endl;
        
        system_manager.update(0.016f);
        
        if (camera.get_target_entity().has_value()) {
            std::cout << "Camera now following Entity ID: " << camera.get_target_entity().value() << std::endl;
        }
    }
    
    std::cout << "\nTesting invalid possession (Agent 9)..." << std::endl;
    if (!possession_system->possess_agent(9)) {
        std::cout << "Correctly rejected invalid agent number" << std::endl;
    }
    
    std::cout << "\nReleasing possession..." << std::endl;
    possession_system->release_possession();
    if (!possession_system->get_possessed_entity().has_value()) {
        std::cout << "Successfully released possession" << std::endl;
    }
    
    if (!camera.get_target_entity().has_value()) {
        std::cout << "Camera stopped following" << std::endl;
    }
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    
    engine.shutdown();
    
    return 0;
}