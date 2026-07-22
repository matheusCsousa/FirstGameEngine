#include "Scene.hpp"

#include "../Graphics/Renderer/Renderer.hpp"
#include "../Graphics/Renderer/Texture/Texture.hpp"
#include "../Window/Window.hpp"
#include <Input.hpp>
#include <Physics.hpp>

namespace Core {

Scene::Scene(std::unique_ptr<Core::Graphics::Camera> camera)
    : m_camera(std::move(camera)) {
    m_assetManager = std::make_shared<Core::AssetManager>();
}

Core::Entity* Scene::createEntity(
    std::shared_ptr<Core::Graphics::Mesh> mesh,
    std::shared_ptr<Core::Render::Shader> shader
) {
    m_entities.push_back(
        std::make_unique<Core::Entity>(mesh, shader)
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
        
        enty->getShader()->setVec3("viewPos", m_camera->getPosition());
        enty->getShader()->setVec3("lightPos", glm::vec3(3.0f, 3.0f, 5.0f));
        enty->getShader()->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        for (auto& tex : enty->getTextures()) {
            tex->bind();
        }

        Core::Graphics::Renderer::Draw(*enty->getMesh(), *enty->getShader(), enty->getModel());
    }
}

void Scene::update() {
    m_camera->update();
    Core::Physics::PhysicsEngine::update(Core::Input::getDeltaTime(), m_entities);
}
}
