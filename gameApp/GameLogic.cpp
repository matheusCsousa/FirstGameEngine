#include "GameLogic.hpp"

#include <GLFW/glfw3.h>

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "../core/Input/Input.hpp"
#include "../core/Window/Window.hpp"
#include "../core/Graphics/Renderer/Renderer.hpp"
#include "../core/Graphics/Mesh/Cube.hpp"

struct GameLogic::object {
    Core::Graphics::Mesh mesh;
    Core::Render::Shader shader;
    glm::mat4 model;
};

GameLogic::GameLogic(std::shared_ptr<Core::Window> window)
    : m_window(window) {
    std::cout << "GameLogic started!" << std::endl;
    Core::Graphics::Mesh mesh = Core::Graphics::Cube::create();
    Core::Render::Shader shader("gameApp/shader/shader.vert", "gameApp/shader/shader.frag");
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    m_objects.push_back(object{mesh, shader, model});
}

void GameLogic::onUpdate() {
    if (Core::Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        std::cout << "Escape pressed!" << std::endl;
        m_window->requestClose();
    }
    if (Core::Input::isKeyPressed(GLFW_KEY_F)) {
        std::cout << "F pressed!" << std::endl;
        Core::Input::toggleMouseMode();
    }
    if (Core::Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        std::cout << "Left mouse button pressed!" << std::endl;
    }

    static float rotation = 0.0f;
    rotation += 0.01f;

    m_objects[0].model = glm::rotate(
        glm::mat4(1.0f),
        rotation,
        glm::vec3(0.5f, 1.0f, 0.0f)
    );
}

void GameLogic::onRender() {
    Core::Graphics::Renderer::Clear();
    for (auto &obj : m_objects) {
        Core::Graphics::Renderer::Draw(obj.mesh, obj.shader, obj.model);
    }
}

GameLogic::~GameLogic() {
    std::cout << "GameLogic stopped!" << std::endl;
}
