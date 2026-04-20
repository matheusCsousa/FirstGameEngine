#include "Scene.hpp"

#include "../Graphics/Renderer/Renderer.hpp"
#include "../Window/Window.hpp"

namespace Core {

Scene::Scene(std::unique_ptr<Core::Graphics::Camera> camera)
    : m_camera(std::move(camera)) {
}

Core::Entity* Scene::createEntity(
    Core::Graphics::Mesh mesh,
    glm::mat4 model,
    Core::Render::Shader shader
) {
    m_entities.push_back(
        std::make_unique<Core::Entity>(mesh, model, shader)
    );

    return m_entities.back().get();
}

std::vector<std::unique_ptr<Core::Entity>>& Scene::getEntities() {
    return m_entities;
}

Core::Graphics::Camera* Scene::getCamera() {
    return m_camera.get();
}

void Scene::render(Window* window) {
    for (auto &enty : m_entities) {
        enty->getShader()->activate();
        enty->getShader()->setMat4("view", m_camera->calcViewMatrix());
        enty->getShader()->setMat4("projection", m_camera->calcProjectionMatrix(window->aspectRatio()));
        Core::Graphics::Renderer::Draw(*enty->getMesh(), *enty->getShader(), enty->getModel());
    }
}
}
