#include "Entity.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace Core {

Entity::Entity(std::shared_ptr<Core::Graphics::Mesh> mesh, std::shared_ptr<Core::Render::Shader> shader)
    : m_mesh(mesh), m_shader(shader) {}

glm::mat4 Core::Entity::getModel() const {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);

    model = glm::rotate(
        model,
        glm::radians(rotation.x),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(rotation.y),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(rotation.z),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    model = glm::scale(model, scale);

    return model;
}

std::shared_ptr<Core::Graphics::Mesh> Entity::getMesh() {
    return m_mesh;
}
std::shared_ptr<Core::Render::Shader> Entity::getShader() {
    return m_shader;
}

void Entity::addTexture(std::shared_ptr<Core::Render::Texture> texture) {
    m_textures.push_back(texture);
}

const std::vector<std::shared_ptr<Core::Render::Texture>>& Entity::getTextures() const {
    return m_textures;
}

Core::Collision::AABB Entity::getLocalAABB() const {
    return m_localAABB;
}

Core::Collision::AABB Entity::getWorldAABB() const {
    return m_localAABB.transform(getModel());
}

void Entity::setLocalAABB(const Core::Collision::AABB& aabb) {
    m_localAABB = aabb;
}
}
