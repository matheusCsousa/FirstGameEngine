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

Core::Graphics::Camera camera;
Core::Scene scene(camera);

GameLogic::GameLogic(std::shared_ptr<Core::Window> window)
    : m_window(window) {
    std::cout << "GameLogic started!" << std::endl;

    Core::Graphics::Mesh mesh = Core::Graphics::Cube::createMesh();
    Core::Render::Shader shader("gameApp/shader/shader.vert", "gameApp/shader/shader.frag");
    glm::mat4 model = glm::mat4(1.0f);

    auto poly = scene.createEntity(mesh, model, shader);

    camera = Core::Graphics::Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.1f, 100.0f);
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

    scene.getEntities()[0]->position.x = 2.0f;
    scene.getEntities()[0]->rotation.y += 1.0f;
}

void GameLogic::onRender() {
    Core::Graphics::Renderer::Clear();
    scene.getEntities()[0]->getShader()->activate();
    scene.getEntities()[0]->getShader()->setMat4("view", camera.calcViewMatrix());
    scene.getEntities()[0]->getShader()->setMat4("projection", camera.calcProjectionMatrix(m_window->aspectRatio()));
    Core::Graphics::Renderer::Draw(*scene.getEntities()[0]->getMesh(), *scene.getEntities()[0]->getShader(), scene.getEntities()[0]->getModel());
}

GameLogic::~GameLogic() {
    std::cout << "GameLogic stopped!" << std::endl;
}
