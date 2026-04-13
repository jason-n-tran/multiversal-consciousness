#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <typeindex>
#include <unordered_map>

class EntityManager;
class ComponentRegistry;

class ISystem {
public:
    virtual ~ISystem() = default;
    
    virtual void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {}
    
    virtual void update(float delta_time) = 0;
    
    virtual void shutdown() {}
    
protected:
    EntityManager* entity_manager_ = nullptr;
    ComponentRegistry* component_registry_ = nullptr;
    
    void set_ecs_references(EntityManager& entity_manager, ComponentRegistry& component_registry) {
        entity_manager_ = &entity_manager;
        component_registry_ = &component_registry;
    }
    
    friend class SystemManager;
};

class IRenderSystem : public ISystem {
public:
    virtual void render(SDL_Renderer* renderer) = 0;
};

class SystemManager {
private:
    std::vector<std::unique_ptr<ISystem>> systems_;
    std::vector<IRenderSystem*> render_systems_;
    std::unordered_map<std::type_index, ISystem*> system_map_;
    
    EntityManager* entity_manager_;
    ComponentRegistry* component_registry_;
    
    bool is_initialized_;
    
public:
    SystemManager(EntityManager& entity_manager, ComponentRegistry& component_registry);
    
    ~SystemManager();
    
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;
    
    SystemManager(SystemManager&&) noexcept = default;
    SystemManager& operator=(SystemManager&&) noexcept = default;
    
    template<typename T>
    T* register_system(std::unique_ptr<T> system) {
        static_assert(std::is_base_of_v<ISystem, T>, "T must derive from ISystem");
        
        T* system_ptr = system.get();
        
        system->set_ecs_references(*entity_manager_, *component_registry_);
        
        if (is_initialized_) {
            system->initialize(*entity_manager_, *component_registry_);
        }
        
        if constexpr (std::is_base_of_v<IRenderSystem, T>) {
            render_systems_.push_back(static_cast<IRenderSystem*>(system_ptr));
        }
        
        system_map_[std::type_index(typeid(T))] = system_ptr;
        
        systems_.push_back(std::move(system));
        
        return system_ptr;
    }
    
    template<typename T>
    T* get_system() {
        static_assert(std::is_base_of_v<ISystem, T>, "T must derive from ISystem");
        
        auto it = system_map_.find(std::type_index(typeid(T)));
        if (it != system_map_.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }
    
    template<typename T>
    const T* get_system() const {
        static_assert(std::is_base_of_v<ISystem, T>, "T must derive from ISystem");
        
        auto it = system_map_.find(std::type_index(typeid(T)));
        if (it != system_map_.end()) {
            return static_cast<const T*>(it->second);
        }
        return nullptr;
    }
    
    void initialize();
    
    void update(float delta_time);
    
    void render(SDL_Renderer* renderer);
    
    void shutdown();
    
    size_t get_system_count() const {
        return systems_.size();
    }
    
    size_t get_render_system_count() const {
        return render_systems_.size();
    }
    
    bool is_initialized() const {
        return is_initialized_;
    }
};