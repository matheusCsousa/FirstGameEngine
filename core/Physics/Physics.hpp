#pragma once

#include <vector>
#include <memory>
#include <Entity.hpp>

namespace Core::Physics {

class PhysicsEngine {
public:
    static void update(float dt, std::vector<std::unique_ptr<Core::Entity>>& entities);
};

}
