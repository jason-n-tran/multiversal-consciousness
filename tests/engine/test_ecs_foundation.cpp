#include <catch2/catch_all.hpp>
#include "engine/EntityManager.h"
#include "engine/ComponentRegistry.h"
#include "engine/System.h"
#include <memory>

// Test component types
struct TestTransform {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
};

struct TestHealth {
    int current = 100;
    int maximum = 100;
};

struct TestName {
    std::string name;
};

// Test system
class TestSystem : public ISystem {
public:
    int update_count = 0;
    bool initialized = false;
    bool shutdown_called = false;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override {
        initialized = true;
    }
    
    void update(float delta_time) override {
        update_count++;
    }
    
    void shutdown() override {
        shutdown_called = true;
    }
};

// Test render system
class TestRenderSystem : public IRenderSystem {
public:
    int update_count = 0;
    int render_count = 0;
    bool initialized = false;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override {
        initialized = true;
    }
    
    void update(float delta_time) override {
        update_count++;
    }
    
    void render(SDL_Renderer* renderer) override {
        render_count++;
    }
};

TEST_CASE("EntityManager basic functionality", "[ecs][entity]") {
    EntityManager entity_manager;
    
    SECTION("Initial state") {
        REQUIRE(entity_manager.get_active_count() == 0);
        REQUIRE(entity_manager.get_recycled_count() == 0);
    }
    
    SECTION("Entity creation") {
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        REQUIRE(entity1 != INVALID_ENTITY);
        REQUIRE(entity2 != INVALID_ENTITY);
        REQUIRE(entity1 != entity2);
        REQUIRE(entity_manager.get_active_count() == 2);
        
        REQUIRE(entity_manager.is_valid(entity1));
        REQUIRE(entity_manager.is_valid(entity2));
        REQUIRE_FALSE(entity_manager.is_valid(INVALID_ENTITY));
    }
    
    SECTION("Entity destruction and recycling") {
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        REQUIRE(entity_manager.destroy_entity(entity1));
        REQUIRE(entity_manager.get_active_count() == 1);
        REQUIRE(entity_manager.get_recycled_count() == 1);
        REQUIRE_FALSE(entity_manager.is_valid(entity1));
        REQUIRE(entity_manager.is_valid(entity2));
        
        // Create new entity - should reuse recycled ID
        EntityID entity3 = entity_manager.create_entity();
        REQUIRE(entity3 == entity1);  // Should reuse the recycled ID
        REQUIRE(entity_manager.get_active_count() == 2);
        REQUIRE(entity_manager.get_recycled_count() == 0);
    }
    
    SECTION("Invalid entity operations") {
        REQUIRE_FALSE(entity_manager.destroy_entity(INVALID_ENTITY));
        REQUIRE_FALSE(entity_manager.destroy_entity(999));  // Non-existent entity
    }
    
    SECTION("Clear functionality") {
        entity_manager.create_entity();
        entity_manager.create_entity();
        EntityID entity3 = entity_manager.create_entity();
        entity_manager.destroy_entity(entity3);
        
        REQUIRE(entity_manager.get_active_count() == 2);
        REQUIRE(entity_manager.get_recycled_count() == 1);
        
        entity_manager.clear();
        REQUIRE(entity_manager.get_active_count() == 0);
        REQUIRE(entity_manager.get_recycled_count() == 0);
    }
}

TEST_CASE("ComponentRegistry basic functionality", "[ecs][component]") {
    ComponentRegistry registry;
    EntityManager entity_manager;
    
    EntityID entity1 = entity_manager.create_entity();
    EntityID entity2 = entity_manager.create_entity();
    
    SECTION("Component addition and retrieval") {
        TestTransform transform{10.0f, 20.0f, 45.0f};
        registry.add_component<TestTransform>(entity1, transform);
        
        REQUIRE(registry.has_component<TestTransform>(entity1));
        REQUIRE_FALSE(registry.has_component<TestTransform>(entity2));
        REQUIRE_FALSE(registry.has_component<TestHealth>(entity1));
        
        TestTransform* retrieved = registry.get_component<TestTransform>(entity1);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->x == 10.0f);
        REQUIRE(retrieved->y == 20.0f);
        REQUIRE(retrieved->rotation == 45.0f);
        
        REQUIRE(registry.get_component<TestTransform>(entity2) == nullptr);
    }
    
    SECTION("Component update") {
        TestTransform transform{10.0f, 20.0f, 45.0f};
        registry.add_component<TestTransform>(entity1, transform);
        
        // Update component
        TestTransform new_transform{30.0f, 40.0f, 90.0f};
        registry.add_component<TestTransform>(entity1, new_transform);
        
        TestTransform* retrieved = registry.get_component<TestTransform>(entity1);
        REQUIRE(retrieved->x == 30.0f);
        REQUIRE(retrieved->y == 40.0f);
        REQUIRE(retrieved->rotation == 90.0f);
    }
    
    SECTION("Multiple component types") {
        TestTransform transform{10.0f, 20.0f, 45.0f};
        TestHealth health{80, 100};
        TestName name{"TestEntity"};
        
        registry.add_component<TestTransform>(entity1, transform);
        registry.add_component<TestHealth>(entity1, health);
        registry.add_component<TestName>(entity1, name);
        
        REQUIRE(registry.has_component<TestTransform>(entity1));
        REQUIRE(registry.has_component<TestHealth>(entity1));
        REQUIRE(registry.has_component<TestName>(entity1));
        
        REQUIRE(registry.get_component<TestTransform>(entity1)->x == 10.0f);
        REQUIRE(registry.get_component<TestHealth>(entity1)->current == 80);
        REQUIRE(registry.get_component<TestName>(entity1)->name == "TestEntity");
    }
    
    SECTION("Component removal") {
        TestTransform transform{10.0f, 20.0f, 45.0f};
        TestHealth health{80, 100};
        
        registry.add_component<TestTransform>(entity1, transform);
        registry.add_component<TestHealth>(entity1, health);
        
        REQUIRE(registry.remove_component<TestTransform>(entity1));
        REQUIRE_FALSE(registry.has_component<TestTransform>(entity1));
        REQUIRE(registry.has_component<TestHealth>(entity1));
        
        REQUIRE_FALSE(registry.remove_component<TestTransform>(entity1));  // Already removed
        REQUIRE_FALSE(registry.remove_component<TestName>(entity1));       // Never had this component
    }
    
    SECTION("Remove all components from entity") {
        TestTransform transform{10.0f, 20.0f, 45.0f};
        TestHealth health{80, 100};
        
        registry.add_component<TestTransform>(entity1, transform);
        registry.add_component<TestHealth>(entity1, health);
        registry.add_component<TestTransform>(entity2, transform);
        
        registry.remove_all_components(entity1);
        
        REQUIRE_FALSE(registry.has_component<TestTransform>(entity1));
        REQUIRE_FALSE(registry.has_component<TestHealth>(entity1));
        REQUIRE(registry.has_component<TestTransform>(entity2));  // Should not affect other entities
    }
}

TEST_CASE("SystemManager functionality", "[ecs][system]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    system_manager.initialize();  // Initialize the system manager
    
    SECTION("System registration and retrieval") {
        auto test_system = std::make_unique<TestSystem>();
        TestSystem* system_ptr = test_system.get();
        
        TestSystem* registered_ptr = system_manager.register_system(std::move(test_system));
        REQUIRE(registered_ptr == system_ptr);
        REQUIRE(registered_ptr->initialized);  // Should be initialized immediately
        
        TestSystem* retrieved_ptr = system_manager.get_system<TestSystem>();
        REQUIRE(retrieved_ptr == system_ptr);
        
        REQUIRE(system_manager.get_system_count() == 1);
        REQUIRE(system_manager.get_render_system_count() == 0);
    }
    
    SECTION("Render system registration") {
        auto render_system = std::make_unique<TestRenderSystem>();
        TestRenderSystem* system_ptr = render_system.get();
        
        TestRenderSystem* registered_ptr = system_manager.register_system(std::move(render_system));
        REQUIRE(registered_ptr == system_ptr);
        REQUIRE(registered_ptr->initialized);
        
        REQUIRE(system_manager.get_system_count() == 1);
        REQUIRE(system_manager.get_render_system_count() == 1);
    }
    
    SECTION("System update") {
        auto test_system = std::make_unique<TestSystem>();
        TestSystem* system_ptr = system_manager.register_system(std::move(test_system));
        
        REQUIRE(system_ptr->update_count == 0);
        
        system_manager.update(0.016f);
        REQUIRE(system_ptr->update_count == 1);
        
        system_manager.update(0.016f);
        REQUIRE(system_ptr->update_count == 2);
    }
    
    SECTION("System shutdown") {
        auto test_system = std::make_unique<TestSystem>();
        TestSystem* system_ptr = system_manager.register_system(std::move(test_system));
        
        REQUIRE_FALSE(system_ptr->shutdown_called);
        
        system_manager.shutdown();
        REQUIRE(system_ptr->shutdown_called);
        REQUIRE(system_manager.get_system_count() == 0);
    }
}

TEST_CASE("ECS Integration", "[ecs][integration]") {
    EntityManager entity_manager;
    ComponentRegistry component_registry;
    SystemManager system_manager(entity_manager, component_registry);
    system_manager.initialize();  // Initialize the system manager
    
    SECTION("Complete ECS workflow") {
        // Create entities
        EntityID entity1 = entity_manager.create_entity();
        EntityID entity2 = entity_manager.create_entity();
        
        // Add components
        TestTransform transform1{10.0f, 20.0f, 0.0f};
        TestTransform transform2{30.0f, 40.0f, 90.0f};
        TestHealth health{100, 100};
        
        component_registry.add_component<TestTransform>(entity1, transform1);
        component_registry.add_component<TestHealth>(entity1, health);
        component_registry.add_component<TestTransform>(entity2, transform2);
        
        // Register systems
        auto test_system = std::make_unique<TestSystem>();
        TestSystem* system_ptr = system_manager.register_system(std::move(test_system));
        
        // Verify integration
        REQUIRE(entity_manager.get_active_count() == 2);
        REQUIRE(component_registry.has_component<TestTransform>(entity1));
        REQUIRE(component_registry.has_component<TestTransform>(entity2));
        REQUIRE(component_registry.has_component<TestHealth>(entity1));
        REQUIRE_FALSE(component_registry.has_component<TestHealth>(entity2));
        
        REQUIRE(system_ptr->initialized);
        
        // Update systems
        system_manager.update(0.016f);
        REQUIRE(system_ptr->update_count == 1);
        
        // Cleanup entity
        entity_manager.destroy_entity(entity1);
        component_registry.remove_all_components(entity1);
        
        REQUIRE(entity_manager.get_active_count() == 1);
        REQUIRE_FALSE(component_registry.has_component<TestTransform>(entity1));
        REQUIRE(component_registry.has_component<TestTransform>(entity2));
    }
}