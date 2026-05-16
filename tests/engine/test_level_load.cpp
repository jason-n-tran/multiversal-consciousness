#include "src/engine/LevelLoader.h"
#include "src/engine/EntityManager.h"
#include "src/engine/ComponentRegistry.h"
#include <iostream>

int main() {
    std::cout << "Testing level loading..." << std::endl;
    
    auto entity_manager = std::make_unique<EntityManager>();
    auto component_registry = std::make_unique<ComponentRegistry>();
    
    LevelLoader loader(entity_manager.get(), component_registry.get());
    
    std::cout << "Loading level file..." << std::endl;
    LevelData level = loader.load_level_from_file("levels/ventilation_lockdown.level");
    
    std::cout << "Level name: '" << level.name << "'" << std::endl;
    std::cout << "Level description: '" << level.description << "'" << std::endl;
    std::cout << "Agents: " << level.agents.size() << std::endl;
    std::cout << "Quantum nodes: " << level.quantum_nodes.size() << std::endl;
    std::cout << "Environment: " << level.environment.size() << std::endl;
    std::cout << "Conditions: " << level.completion_conditions.size() << std::endl;
    
    return 0;
}