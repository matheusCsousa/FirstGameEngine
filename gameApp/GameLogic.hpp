#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>

#include "../core/Logic/Logic.hpp"

namespace Core {
    class Window;
    class Entity;
    class Scene;
    namespace Render {
        class Shader;
    }
}

class GameLogic : public Core::Logic {
private:
    std::shared_ptr<Core::Window> m_window;
    std::vector<std::unique_ptr<Core::Scene>> m_scenes;

    Core::Entity* m_cube = nullptr;
    std::shared_ptr<Core::Render::Shader> m_shader;

public:
    GameLogic(std::shared_ptr<Core::Window> window);
    ~GameLogic();

    void onUpdate() override;
    void onRender() override;
    void onEvent(Core::Event& event) override;
};
