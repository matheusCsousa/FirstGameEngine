#pragma once

#include <glm/glm.hpp>
#include <algorithm>

namespace Core::Collision {

struct AABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};

    // Check collision with another AABB
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    // Apply transformation matrix to AABB
    AABB transform(const glm::mat4& matrix) const {
        glm::vec3 corners[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {min.x, max.y, min.z}, {max.x, max.y, min.z},
            {min.x, min.y, max.z}, {max.x, min.y, max.z},
            {min.x, max.y, max.z}, {max.x, max.y, max.z}
        };

        glm::vec3 newMin = glm::vec3(matrix * glm::vec4(corners[0], 1.0f));
        glm::vec3 newMax = newMin;

        for (int i = 1; i < 8; ++i) {
            glm::vec3 transformed = glm::vec3(matrix * glm::vec4(corners[i], 1.0f));
            newMin = glm::min(newMin, transformed);
            newMax = glm::max(newMax, transformed);
        }

        return {newMin, newMax};
    }
};

}
