#pragma once

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include "../Graphics/Renderer/Shader/Shader.hpp"
#include "../Graphics/Mesh/Mesh.hpp"

namespace Core {
class Entity {
private:
    Core::Graphics::Mesh m_mesh;
    glm::mat4 m_model;
    Core::Render::Shader m_shader;

public:
    Entity();
    Entity( Core::Graphics::Mesh mesh, glm::mat4 model, Core::Render::Shader shader);
    ~Entity() = default;

    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    void update();
    glm::mat4 getModel() const;
    Core::Graphics::Mesh* getMesh();
    Core::Render::Shader* getShader();

};
}
