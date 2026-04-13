#include "System.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"

SystemManager::SystemManager(EntityManager& entity_manager, ComponentRegistry& component_registry)
    : entity_manager_(&entity_manager)
    , component_registry_(&component_registry)
    , is_initialized_(false) {
}

SystemManager::~SystemManager() {
    shutdown();
}

void SystemManager::initialize() {
    if (is_initialized_) {
        return;
    }
    
    for (auto& system : systems_) {
        system->initialize(*entity_manager_, *component_registry_);
    }
    
    is_initialized_ = true;
}

void SystemManager::update(float delta_time) {
    for (auto& system : systems_) {
        system->update(delta_time);
    }
}

void SystemManager::render(SDL_Renderer* renderer) {
    for (auto* render_system : render_systems_) {
        render_system->render(renderer);
    }
}

void SystemManager::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    for (auto it = systems_.rbegin(); it != systems_.rend(); ++it) {
        (*it)->shutdown();
    }
    
    systems_.clear();
    render_systems_.clear();
    system_map_.clear();
    
    is_initialized_ = false;
}