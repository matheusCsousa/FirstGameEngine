#include "Scene.hpp"

#include "../Graphics/Renderer/Renderer.hpp"
#include "../Graphics/Renderer/Texture/Texture.hpp"
#include "../Window/Window.hpp"
#include <Input.hpp>
#include <Physics.hpp>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace Core {

Scene::Scene(std::unique_ptr<Core::Graphics::Camera> camera)
    : m_camera(std::move(camera)) {
    m_assetManager = std::make_shared<Core::AssetManager>();
}

Core::Entity* Scene::createEntity(
    std::shared_ptr<Core::Graphics::Mesh> mesh,
    std::shared_ptr<Core::Render::Shader> shader, EntityId requestedId
) {
    const auto id = requestedId ? requestedId : m_nextId;
    if (id == std::numeric_limits<EntityId>::max() || findEntity(id))
        throw std::invalid_argument("Entity ID is reserved or already in use");
    m_entities.push_back(
        std::make_unique<Core::Entity>(mesh, shader)
    );

    m_entities.back()->m_id = id;
    m_nextId = std::max(m_nextId, id + 1);
    return m_entities.back().get();
}

std::vector<std::unique_ptr<Core::Entity>>& Scene::getEntities() {
    return m_entities;
}

Core::Graphics::Camera* Scene::getCamera() {
    return m_camera.get();
}

Entity* Scene::findEntity(EntityId id) {
    for (auto& entity : m_entities) if (entity->id() == id) return entity.get();
    return nullptr;
}

bool Scene::removeEntity(EntityId id) {
    return std::erase_if(m_entities, [id](const auto& entity) { return entity->id() == id; }) != 0;
}

Entity* Scene::duplicateEntity(EntityId id) {
    auto* source = findEntity(id);
    if (!source) return nullptr;
    auto* copy = createEntity(source->getMesh(), source->getShader());
    copy->name = source->name + " Copy";
    copy->position = source->position; copy->rotation = source->rotation; copy->scale = source->scale;
    copy->velocity = source->velocity; copy->acceleration = source->acceleration;
    copy->mass = source->mass; copy->useGravity = source->useGravity;
    copy->setLocalAABB(source->getLocalAABB());
    for (auto& texture : source->getTextures()) copy->addTexture(texture);
    return copy;
}

void Scene::render(Window* window) { render(window->aspectRatio()); }

void Scene::render(float aspectRatio) {
    const auto view = m_camera->calcViewMatrix();
    const auto projection = m_camera->calcProjectionMatrix(aspectRatio);
    for (auto &enty : m_entities) {
        enty->getShader()->activate();
        enty->getShader()->setMat4("view", view);
        enty->getShader()->setMat4("projection", projection);
        
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
