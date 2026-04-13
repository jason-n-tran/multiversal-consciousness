#include "EntityManager.h"
#include <algorithm>

EntityManager::EntityManager() 
    : next_id_(1) {}

EntityID EntityManager::create_entity() {
    EntityID new_id;
    
    if (!available_ids_.empty()) {
        new_id = available_ids_.front();
        available_ids_.pop();
    } else {
        new_id = next_id_;
        ++next_id_;
        
        if (next_id_ == INVALID_ENTITY) {
            next_id_ = 1;
        }
    }
    
    active_entities_.insert(new_id);
    
    return new_id;
}

bool EntityManager::destroy_entity(EntityID entity) {
    if (entity == INVALID_ENTITY || active_entities_.find(entity) == active_entities_.end()) {
        return false;
    }
    
    active_entities_.erase(entity);
    
    available_ids_.push(entity);
    
    return true;
}

bool EntityManager::is_valid(EntityID entity) const {
    return entity != INVALID_ENTITY && 
           active_entities_.find(entity) != active_entities_.end();
}

size_t EntityManager::get_active_count() const {
    return active_entities_.size();
}

size_t EntityManager::get_recycled_count() const {
    return available_ids_.size();
}

void EntityManager::clear() {
    active_entities_.clear();
    
    std::queue<EntityID> empty_queue;
    available_ids_.swap(empty_queue);
    
    next_id_ = 1;
}

std::unordered_set<EntityID> EntityManager::get_active_entities() const {
    return active_entities_;
}