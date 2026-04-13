#pragma once

#include <cstdint>
#include <queue>
#include <unordered_set>
#include <memory>


using EntityID = uint32_t;

constexpr EntityID INVALID_ENTITY = 0;


class EntityManager {
private:
    std::queue<EntityID> available_ids_;
    std::unordered_set<EntityID> active_entities_;
    EntityID next_id_;
    
public:
    EntityManager();
    
    ~EntityManager() = default;
    
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;
    
    EntityID create_entity();
    
    bool destroy_entity(EntityID entity);
    
    bool is_valid(EntityID entity) const;
    
    size_t get_active_count() const;
    
    size_t get_recycled_count() const;
    
    void clear();
    
    std::unordered_set<EntityID> get_active_entities() const;
};