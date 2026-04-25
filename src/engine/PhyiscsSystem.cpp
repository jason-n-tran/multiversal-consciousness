#include "PhysicsSystem.h"
#include <algorithm>
#include <unordered_map>
#include <iostream>

bool PhysicsSystem::is_valid_physics_state(const PhysicsComponent& physics) const {
    return std::isfinite(physics.velocity_x) && 
           std::isfinite(physics.velocity_y) &&
           std::isfinite(physics.acceleration_x) &&
           std::isfinite(physics.acceleration_y) &&
           physics.mass > EPSILON;
}

void PhysicsSystem::apply_gravity(PhysicsComponent& physics, float delta_time) {
    // Only apply gravity if enabled and entity is not grounded
    if (physics.apply_gravity && !physics.is_grounded) {
        physics.velocity_y += physics.acceleration_y * delta_time;
    }
}

void PhysicsSystem::update_velocity(PhysicsComponent& physics, float delta_time) {
    physics.velocity_x += physics.acceleration_x * delta_time;
    if (physics.is_grounded && std::abs(physics.velocity_x) > EPSILON) {
        float friction_force = physics.friction * delta_time;
        if (physics.velocity_x > 0) {
            physics.velocity_x = std::max(0.0f, physics.velocity_x - friction_force);
        } else {
            physics.velocity_x = std::min(0.0f, physics.velocity_x + friction_force);
        }
    }
}

void PhysicsSystem::update_position(Transform& transform, const PhysicsComponent& physics, float delta_time) {
    transform.x += physics.velocity_x * delta_time;
    transform.y += physics.velocity_y * delta_time;
}

AABB PhysicsSystem::create_aabb(const Transform& transform, const BoundingBoxComponent& bounding_box) const {
    return AABB::from_components(transform, bounding_box);
}

CollisionInfo PhysicsSystem::check_collision(EntityID entity1, EntityID entity2) const {
    CollisionInfo collision_info;
    collision_info.other_entity = entity2;
    
    if (!entity_manager_ || !component_registry_) {
        return collision_info;
    }
    
    const Transform* transform1 = component_registry_->get_component<Transform>(entity1);
    const Transform* transform2 = component_registry_->get_component<Transform>(entity2);
    const BoundingBoxComponent* bbox1 = component_registry_->get_component<BoundingBoxComponent>(entity1);
    const BoundingBoxComponent* bbox2 = component_registry_->get_component<BoundingBoxComponent>(entity2);
    
    if (!transform1 || !transform2 || !bbox1 || !bbox2) {
        return collision_info;
    }
    
    AABB aabb1 = create_aabb(*transform1, *bbox1);
    AABB aabb2 = create_aabb(*transform2, *bbox2);
    
    if (!aabb1.intersects(aabb2)) {
        return collision_info;
    }
    
    float overlap_x = aabb1.get_overlap_x(aabb2);
    float overlap_y = aabb1.get_overlap_y(aabb2);
    
    if (overlap_x <= 0.0f || overlap_y <= 0.0f) {
        return collision_info;
    }
    
    collision_info.has_collision = true;
    
    if (overlap_x < overlap_y) {
        collision_info.penetration_x = overlap_x;
        collision_info.penetration_y = 0.0f;
        
        float center1_x = (aabb1.min_x + aabb1.max_x) * 0.5f;
        float center2_x = (aabb2.min_x + aabb2.max_x) * 0.5f;
        collision_info.normal_x = (center1_x < center2_x) ? -1.0f : 1.0f;
        collision_info.normal_y = 0.0f;
    } else {
        collision_info.penetration_x = 0.0f;
        collision_info.penetration_y = overlap_y;
        
        float center1_y = (aabb1.min_y + aabb1.max_y) * 0.5f;
        float center2_y = (aabb2.min_y + aabb2.max_y) * 0.5f;
        collision_info.normal_x = 0.0f;
        collision_info.normal_y = (center1_y < center2_y) ? -1.0f : 1.0f;
    }
    
    return collision_info;
}

std::vector<CollisionInfo> PhysicsSystem::detect_collisions() const {
    std::vector<CollisionInfo> collisions;
    
    if (!entity_manager_ || !component_registry_) {
        return collisions;
    }
    
    const auto* bbox_container = component_registry_->get_all_components<BoundingBoxComponent>();
    if (!bbox_container) {
        return collisions;
    }
    
    const auto& entities = bbox_container->get_entities();
    
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            EntityID entity1 = entities[i];
            EntityID entity2 = entities[j];
            
            if (!entity_manager_->is_valid(entity1) || !entity_manager_->is_valid(entity2)) {
                continue;
            }
            
            CollisionInfo collision = check_collision(entity1, entity2);
            if (collision.has_collision) {
                const PhysicsComponent* physics1 = component_registry_->get_component<PhysicsComponent>(entity1);
                const PhysicsComponent* physics2 = component_registry_->get_component<PhysicsComponent>(entity2);
                
                if (physics1 || physics2) {
                    const BoundingBoxComponent* bbox1 = component_registry_->get_component<BoundingBoxComponent>(entity1);
                    const BoundingBoxComponent* bbox2 = component_registry_->get_component<BoundingBoxComponent>(entity2);
                    std::cout << "Collision detected between entities " << entity1 << " and " << entity2;
                    if (bbox1) std::cout << " (entity1 solid: " << bbox1->is_solid << ")";
                    if (bbox2) std::cout << " (entity2 solid: " << bbox2->is_solid << ")";
                    std::cout << std::endl;
                }
                
                collision.other_entity = entity2;
                collisions.push_back(collision);
                
                CollisionInfo reverse_collision = collision;
                reverse_collision.other_entity = entity1;
                reverse_collision.normal_x = -collision.normal_x;
                reverse_collision.normal_y = -collision.normal_y;
                collisions.push_back(reverse_collision);
            }
        }
    }
    
    return collisions;
}

void PhysicsSystem::resolve_collision_with_slide(EntityID entity, const CollisionInfo& collision) {
    if (!entity_manager_ || !component_registry_ || !collision.has_collision) {
        return;
    }
    
    Transform* transform = component_registry_->get_component<Transform>(entity);
    PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
    const BoundingBoxComponent* bbox = component_registry_->get_component<BoundingBoxComponent>(entity);
    
    if (!transform || !physics || !bbox) {
        return;
    }
    
    const BoundingBoxComponent* other_bbox = component_registry_->get_component<BoundingBoxComponent>(collision.other_entity);
    if (!other_bbox || !other_bbox->is_solid) {
        return; 
    }
    
    const float MIN_PENETRATION = 0.1f;
    if (collision.penetration_x < MIN_PENETRATION && collision.penetration_y < MIN_PENETRATION) {
        return;
    }
    
    std::cout << "Resolving collision: Entity " << entity << " with solid entity " << collision.other_entity 
              << " penetration: (" << collision.penetration_x << ", " << collision.penetration_y << ")"
              << " normal: (" << collision.normal_x << ", " << collision.normal_y << ")" << std::endl;
    
    transform->x += collision.normal_x * collision.penetration_x;
    transform->y += collision.normal_y * collision.penetration_y;
    
    if (std::abs(collision.normal_x) > EPSILON) {
        if (collision.normal_x * physics->velocity_x > 0) {
            physics->velocity_x = 0.0f; 
        }
    }
    
    if (std::abs(collision.normal_y) > EPSILON) {
        if (collision.normal_y < 0) {
            physics->is_grounded = true;
            physics->has_double_jumped = false; 
            if (physics->velocity_y > 0) {
                physics->velocity_y = 0.0f; 
            }
        } else {
            if (physics->velocity_y < 0) {
                physics->velocity_y = 0.0f;
            }
        }
    }
}

void PhysicsSystem::resolve_entity_collisions(EntityID entity, const std::vector<CollisionInfo>& collisions) {
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
    if (physics) {
        physics->is_grounded = false;
    }
    
    for (const auto& collision : collisions) {
        resolve_collision_with_slide(entity, collision);
    }
}

void PhysicsSystem::initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) {
    ISystem::initialize(entity_manager, component_registry);
}

void PhysicsSystem::update(float delta_time) {
    if (!entity_manager_ || !component_registry_) {
        return;
    }
    
    const auto* transform_container = component_registry_->get_all_components<Transform>();
    const auto* physics_container = component_registry_->get_all_components<PhysicsComponent>();
    
    if (!transform_container || !physics_container) {
        return;
    }
    
    const auto& physics_entities = physics_container->get_entities();
    
    for (EntityID entity : physics_entities) {
        if (!entity_manager_->is_valid(entity)) {
            continue;
        }
        
        Transform* transform = component_registry_->get_component<Transform>(entity);
        PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
        
        if (!transform || !physics) {
            continue;
        }
        
        if (!is_valid_physics_state(*physics)) {
            physics->velocity_x = 0.0f;
            physics->velocity_y = 0.0f;
            physics->acceleration_x = 0.0f;
            physics->acceleration_y = 980.0f; 
            continue;
        }
        
        apply_gravity(*physics, delta_time);
        
        update_velocity(*physics, delta_time);
        
        update_position(*transform, *physics, delta_time);
        
        physics->acceleration_x = 0.0f;
    }
    
    std::vector<CollisionInfo> all_collisions = detect_collisions();
    
    std::unordered_map<EntityID, std::vector<CollisionInfo>> collisions_by_entity;
    
    for (size_t i = 0; i < all_collisions.size(); i += 2) {
        if (i + 1 < all_collisions.size()) {
            const auto& collision1 = all_collisions[i];
            const auto& collision2 = all_collisions[i + 1]; 
            EntityID entity1 = collision2.other_entity;
            collisions_by_entity[entity1].push_back(collision1);
            
            EntityID entity2 = collision1.other_entity;
            collisions_by_entity[entity2].push_back(collision2);
        }
    }
    
    for (const auto& [entity, entity_collisions] : collisions_by_entity) {
        const PhysicsComponent* physics = component_registry_->get_component<PhysicsComponent>(entity);
        if (physics && entity_manager_->is_valid(entity)) {
            resolve_entity_collisions(entity, entity_collisions);
        }
    }
}

void PhysicsSystem::shutdown() {
}