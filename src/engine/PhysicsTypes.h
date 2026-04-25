#pragma once

#include "Components.h"
#include <cmath>
#include <algorithm>


struct CollisionInfo {
    bool has_collision{false};
    float penetration_x{0.0f};
    float penetration_y{0.0f};
    float normal_x{0.0f};
    float normal_y{0.0f};
    EntityID other_entity{0};
};


struct AABB {
    float min_x, min_y, max_x, max_y;

    bool intersects(const AABB& other) const {
        return (min_x < other.max_x && max_x > other.min_x &&
                min_y < other.max_y && max_y > other.min_y);
    }

    float get_overlap_x(const AABB& other) const {
        return std::min(max_x, other.max_x) - std::max(min_x, other.min_x);
    }
    
    float get_overlap_y(const AABB& other) const {
        return std::min(max_y, other.max_y) - std::max(min_y, other.min_y);
    }

    static AABB from_components(const Transform& transform, const BoundingBoxComponent& bbox) {
        float half_width = bbox.width * 0.5f;
        float half_height = bbox.height * 0.5f;
        float center_x = transform.x + bbox.offset_x;
        float center_y = transform.y + bbox.offset_y;
        
        return AABB{
            center_x - half_width,
            center_y - half_height,
            center_x + half_width,
            center_y + half_height
        };
    }
};