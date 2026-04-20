#include "GameLogic.hpp"

#include <GLFW/glfw3.h>

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "../core/Input/Input.hpp"
#include "../core/Window/Window.hpp"
#include "../core/Graphics/Renderer/Renderer.hpp"
#include "../core/Graphics/Mesh/Cube.hpp"
#include "../core/Graphics/Camera/Camera.hpp"
#include "../core/Entity/Entity.hpp"
#include "../core/Scene/Scene.hpp"
#include "Camera/freeCam.hpp"

GameLogic::GameLogic(std::shared_ptr<Core::Window> window)
    : m_window(window) {
    std::cout << "GameLogic started!" << std::endl;

    Core::Graphics::Mesh mesh = Core::Graphics::Cube::createMesh();
    Core::Render::Shader shader("gameApp/shader/shader.vert", "gameApp/shader/shader.frag");
    glm::mat4 model = glm::mat4(1.0f);

    // Core::Graphics::Camera camera = Core::Graphics::Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.1f, 100.0f);
    auto camera = std::make_unique<FreeCam>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.1f, 100.0f);
    auto scene = std::make_unique<Core::Scene>(std::move(camera));

    auto cube = scene->createEntity(mesh, model, shader);
    cube->position.x = 2.0f;

    m_scenes.push_back(std::move(scene));
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


    m_scenes[0]->getCamera()->update();
    m_scenes[0]->getEntities()[0]->rotation.y += 1.0f;
    m_scenes[0]->getEntities()[0]->rotation.z += 0.5f;
}

void GameLogic::onRender() {
    Core::Graphics::Renderer::Clear();
    for (auto &scene : m_scenes) {
        scene->render(m_window.get());
    }
}

GameLogic::~GameLogic() {
    std::cout << "GameLogic stopped!" << std::endl;
}
