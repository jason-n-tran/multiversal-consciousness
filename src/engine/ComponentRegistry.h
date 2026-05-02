#pragma once

#include "EntityManager.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cassert>

class IComponentContainer {
public:
    virtual ~IComponentContainer() = default;
    
    virtual bool remove_component(EntityID entity) = 0;
    
    virtual bool has_component(EntityID entity) const = 0;

    virtual void clear() = 0;

    virtual size_t size() const = 0;
};

template<typename T>
class ComponentContainer : public IComponentContainer {
private:
    std::unordered_map<EntityID, size_t> entity_to_index_;  
    std::vector<T> components_;                          
    std::vector<EntityID> entities_;                     
    
public:
    void add_component(EntityID entity, const T& component) {
        auto it = entity_to_index_.find(entity);
        
        if (it != entity_to_index_.end()) {
            components_[it->second] = component;
        } else {
            size_t index = components_.size();
            entity_to_index_[entity] = index;
            components_.push_back(component);
            entities_.push_back(entity);
        }
    }
    
    void add_component(EntityID entity, T&& component) {
        auto it = entity_to_index_.find(entity);
        
        if (it != entity_to_index_.end()) {
            components_[it->second] = std::move(component);
        } else {
            size_t index = components_.size();
            entity_to_index_[entity] = index;
            components_.push_back(std::move(component));
            entities_.push_back(entity);
        }
    }
    T* get_component(EntityID entity) {
        auto it = entity_to_index_.find(entity);
        if (it != entity_to_index_.end()) {
            return &components_[it->second];
        }
        return nullptr;
    }
    
    const T* get_component(EntityID entity) const {
        auto it = entity_to_index_.find(entity);
        if (it != entity_to_index_.end()) {
            return &components_[it->second];
        }
        return nullptr;
    }
    
    bool remove_component(EntityID entity) override {
        auto it = entity_to_index_.find(entity);
        if (it == entity_to_index_.end()) {
            return false;  
        }
        
        size_t index_to_remove = it->second;
        size_t last_index = components_.size() - 1;
        
        if (index_to_remove != last_index) {
            components_[index_to_remove] = std::move(components_[last_index]);
            entities_[index_to_remove] = entities_[last_index];
            
            entity_to_index_[entities_[index_to_remove]] = index_to_remove;
        }
        
        components_.pop_back();
        entities_.pop_back();
        entity_to_index_.erase(entity);
        
        return true;
    }
    
    bool has_component(EntityID entity) const override {
        return entity_to_index_.find(entity) != entity_to_index_.end();
    }
    
    void clear() override {
        components_.clear();
        entities_.clear();
        entity_to_index_.clear();
    }
    
    size_t size() const override {
        return components_.size();
    }
    
    const std::vector<T>& get_components() const {
        return components_;
    }
    
    const std::vector<EntityID>& get_entities() const {
        return entities_;
    }
};

class ComponentRegistry {
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentContainer>> containers_;
    
    template<typename T>
    ComponentContainer<T>& get_container() {
        std::type_index type_id = std::type_index(typeid(T));
        
        auto it = containers_.find(type_id);
        if (it == containers_.end()) {
            auto container = std::make_unique<ComponentContainer<T>>();
            auto* container_ptr = container.get();
            containers_[type_id] = std::move(container);
            return *container_ptr;
        }
        
        return static_cast<ComponentContainer<T>&>(*it->second);
    }
    
    template<typename T>
    const ComponentContainer<T>* get_container() const {
        std::type_index type_id = std::type_index(typeid(T));
        
        auto it = containers_.find(type_id);
        if (it == containers_.end()) {
            return nullptr;
        }
        
        return static_cast<const ComponentContainer<T>*>(it->second.get());
    }
    
public:
    ComponentRegistry() = default;
    
    ~ComponentRegistry() = default;
    
    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;
    
    ComponentRegistry(ComponentRegistry&&) noexcept = default;
    ComponentRegistry& operator=(ComponentRegistry&&) noexcept = default;
    
    template<typename T>
    void add_component(EntityID entity, const T& component) {
        static_assert(std::is_trivially_copyable_v<T> || std::is_move_constructible_v<T>, 
                      "Component type must be trivially copyable or move constructible");
        
        auto& container = get_container<T>();
        container.add_component(entity, component);
    }
    
    template<typename T>
    void add_component(EntityID entity, T&& component) {
        static_assert(std::is_move_constructible_v<T>, 
                      "Component type must be move constructible");
        
        auto& container = get_container<T>();
        container.add_component(entity, std::move(component));
    }
    
    template<typename T>
    T* get_component(EntityID entity) {
        auto& container = get_container<T>();
        return container.get_component(entity);
    }
    
    template<typename T>
    const T* get_component(EntityID entity) const {
        const auto* container = get_container<T>();
        if (!container) {
            return nullptr;
        }
        return container->get_component(entity);
    }
    
    template<typename T>
    bool remove_component(EntityID entity) {
        std::type_index type_id = std::type_index(typeid(T));
        
        auto it = containers_.find(type_id);
        if (it == containers_.end()) {
            return false;  
        }
        
        return it->second->remove_component(entity);
    }
    
    template<typename T>
    bool has_component(EntityID entity) const {
        const auto* container = get_container<T>();
        if (!container) {
            return false;
        }
        return container->has_component(entity);
    }
    
    void remove_all_components(EntityID entity) {
        for (auto& [type_id, container] : containers_) {
            container->remove_component(entity);
        }
    }
    
    template<typename T>
    const ComponentContainer<T>* get_all_components() const {
        return get_container<T>();
    }
    
    void clear() {
        for (auto& [type_id, container] : containers_) {
            container->clear();
        }
    }
    
    size_t get_component_type_count() const {
        return containers_.size();
    }
};