#pragma once

#include "System.h"
#include "Components.h"
#include "EntityManager.h"
#include "ComponentRegistry.h"
#include "PhysicsTypes.h"
#include <vector>
#include <cmath>

class PhysicsSystem : public ISystem {
private:
    static constexpr float EPSILON = 1e-6f;
    static constexpr int MAX_COLLISION_ITERATIONS = 10;
    
    bool is_valid_physics_state(const PhysicsComponent& physics) const;
    
    void apply_gravity(PhysicsComponent& physics, float delta_time);
    
    void update_velocity(PhysicsComponent& physics, float delta_time);
    
    void update_position(Transform& transform, const PhysicsComponent& physics, float delta_time);
    
    AABB create_aabb(const Transform& transform, const BoundingBoxComponent& bounding_box) const;
    
    CollisionInfo check_collision(EntityID entity1, EntityID entity2) const;
    
    void resolve_collision_with_slide(EntityID entity, const CollisionInfo& collision);
    
    void resolve_entity_collisions(EntityID entity, const std::vector<CollisionInfo>& collisions);
    
public:
    PhysicsSystem() = default;
    
    ~PhysicsSystem() override = default;
    
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;
    
    PhysicsSystem(PhysicsSystem&&) noexcept = default;
    PhysicsSystem& operator=(PhysicsSystem&&) noexcept = default;
    
    void initialize(EntityManager& entity_manager, ComponentRegistry& component_registry) override;
    
    void update(float delta_time) override;
    
    void shutdown() override;
    
    std::vector<CollisionInfo> detect_collisions() const;
};