#include <catch2/catch_test_macros.hpp>
#include "../../src/engine/LevelLoader.h"
#include "../../src/engine/TestChamberSystem.h"
#include "../../src/engine/EntityManager.h"
#include "../../src/engine/ComponentRegistry.h"
#include <memory>
#include <thread>
#include <chrono>

TEST_CASE("Level loading system", "[level]") {
    SECTION("Level file loading test") {
        // Create ECS components
        auto entity_manager = std::make_unique<EntityManager>();
        auto component_registry = std::make_unique<ComponentRegistry>();
        
        LevelLoader loader(entity_manager.get(), component_registry.get());
        
        // Test loading the ventilation lockdown level
        LevelData level = loader.load_level_from_file("levels/ventilation_lockdown.level");
        
        // Just check if the level loaded
        REQUIRE_FALSE(level.name.empty());
        REQUIRE(level.name == "Ventilation Lockdown");
    }
}