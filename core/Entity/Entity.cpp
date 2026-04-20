#include "Entity.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace Core {

Entity::Entity(Core::Graphics::Mesh mesh, glm::mat4 model, Core::Render::Shader shader)
    : m_mesh(mesh), m_model(model), m_shader(shader) {}

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

Core::Graphics::Mesh* Entity::getMesh() {
    return &m_mesh;
}
Core::Render::Shader* Entity::getShader() {
    return &m_shader;
}
}
