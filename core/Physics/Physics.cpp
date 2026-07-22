#include "Physics.hpp"
#include <algorithm>
#include <iostream>

namespace Core::Physics {

struct CollisionInfo {
    glm::vec3 normal{0.0f};
    float penetration{0.0f};
    bool collided{false};
};

CollisionInfo checkCollision(const Core::Collision::AABB& a, const Core::Collision::AABB& b) {
    CollisionInfo info;

    float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
    float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);
    float overlapZ = std::min(a.max.z, b.max.z) - std::max(a.min.z, b.min.z);

    if (overlapX > 0 && overlapY > 0 && overlapZ > 0) {
        info.collided = true;
        
        if (overlapX < overlapY && overlapX < overlapZ) {
            info.penetration = overlapX;
            info.normal = (a.min.x + a.max.x < b.min.x + b.max.x) ? glm::vec3(-1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        } else if (overlapY < overlapX && overlapY < overlapZ) {
            info.penetration = overlapY;
            info.normal = (a.min.y + a.max.y < b.min.y + b.max.y) ? glm::vec3(0.0f, -1.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        } else {
            info.penetration = overlapZ;
            info.normal = (a.min.z + a.max.z < b.min.z + b.max.z) ? glm::vec3(0.0f, 0.0f, -1.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
        }
    }
    return info;
}

void PhysicsEngine::update(float dt, std::vector<std::unique_ptr<Core::Entity>>& entities) {
    if (dt <= 0.0f) return;

    // 1. Integrate entities
    for (auto& entity : entities) {
        if (entity->mass <= 0.0f) continue;

        if (entity->useGravity) {
            entity->acceleration += glm::vec3(0.0f, -9.81f, 0.0f);
        }

        entity->velocity += entity->acceleration * dt;
        entity->position += entity->velocity * dt;
        entity->acceleration = glm::vec3(0.0f);
    }

    // 2. Detect and resolve collisions
    for (int iter = 0; iter < 3; ++iter) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity& a = *entities[i];
                Entity& b = *entities[j];

                if (a.mass <= 0.0f && b.mass <= 0.0f) continue;

                CollisionInfo col = checkCollision(a.getWorldAABB(), b.getWorldAABB());
                if (col.collided) {
                    float totalMass = a.mass + b.mass;
                    
                    float aDisplacementRatio = 0.0f;
                    float bDisplacementRatio = 0.0f;

                    if (a.mass > 0.0f && b.mass <= 0.0f) {
                        aDisplacementRatio = 1.0f;
                    } else if (a.mass <= 0.0f && b.mass > 0.0f) {
                        bDisplacementRatio = 1.0f;
                    } else {
                        aDisplacementRatio = b.mass / totalMass;
                        bDisplacementRatio = a.mass / totalMass;
                    }

                    if (a.mass > 0.0f) {
                        a.position += col.normal * col.penetration * aDisplacementRatio;
                    }
                    if (b.mass > 0.0f) {
                        b.position -= col.normal * col.penetration * bDisplacementRatio;
                    }

                    glm::vec3 relativeVelocity = a.velocity - b.velocity;
                    float velAlongNormal = glm::dot(relativeVelocity, col.normal);

                    if (velAlongNormal < 0.0f) {
                        float restitution = 0.4f;
                        float impulseScalar = -(1.0f + restitution) * velAlongNormal;
                        
                        if (a.mass > 0.0f && b.mass > 0.0f) {
                            impulseScalar /= (1.0f / a.mass + 1.0f / b.mass);
                            a.velocity += (impulseScalar / a.mass) * col.normal;
                            b.velocity -= (impulseScalar / b.mass) * col.normal;
                        } else if (a.mass > 0.0f) {
                            a.velocity += impulseScalar * col.normal;
                        } else if (b.mass > 0.0f) {
                            b.velocity -= impulseScalar * col.normal;
                        }
                    }
                }
            }
        }
    }
}

}
