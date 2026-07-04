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
#include "../core/Entity/Entity.hpp"
#include "../core/Scene/Scene.hpp"
#include "../core/Graphics/Renderer/Texture/Texture.hpp"
#include "Camera/freeCam.hpp"

GameLogic::GameLogic(std::shared_ptr<Core::Window> window)
    : m_window(window) {
    std::cout << "GameLogic started!" << std::endl;
    Core::Input::setMouseMode(false);

    auto camera = std::make_unique<FreeCam>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.1f, 100.0f);
    auto scene = std::make_unique<Core::Scene>(std::move(camera));
    auto am = scene->getAssetManager();

    auto mesh = am->registerMesh("cube_mesh", Core::Graphics::Cube::createMesh());
    m_shader = am->loadShader("basic_shader", "gameApp/shader/shader.vert", "gameApp/shader/shader.frag");
    auto texture = am->loadTexture("brick_tex", "gameApp/textures/brick.jpg", "diffuse", 0);

    texture->texUnit(*m_shader, "tex0", 0);

    m_cube = scene->createEntity(mesh, m_shader);
    m_cube->position.x = 2.0f;
    m_cube->addTexture(texture);

    m_scenes.push_back(std::move(scene));
}

void GameLogic::onUpdate() {
    for (auto &scene : m_scenes) {
        scene->update();
    }

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

    m_cube->rotation.y += 1.0f;
    m_cube->rotation.z += 0.5f;
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

void GameLogic::onEvent(Core::Event& event) {
    Core::EventDispatcher dispatcher(event);
    dispatcher.dispatch<Core::WindowResizeEvent>([](Core::WindowResizeEvent& e) {
        std::cout << "GameLogic Resize Event: " << e.getWidth() << "x" << e.getHeight() << std::endl;
        return false;
    });
    dispatcher.dispatch<Core::KeyPressedEvent>([](Core::KeyPressedEvent& e) {
        std::cout << "GameLogic KeyPressed Event: " << e.getKeyCode() << std::endl;
        return false;
    });
}
