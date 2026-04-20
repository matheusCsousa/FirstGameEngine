#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>

#include "../core/Logic/Logic.hpp"

namespace Core {
    struct Window;
    struct Entity;
    struct Scene;
}

class GameLogic : public Core::Logic {
private:
    std::shared_ptr<Core::Window> m_window;
    std::vector<std::unique_ptr<Core::Scene>> m_scenes;

public:
    GameLogic(std::shared_ptr<Core::Window> window);
    ~GameLogic();

    void onUpdate() override;
    void onRender() override;
};
