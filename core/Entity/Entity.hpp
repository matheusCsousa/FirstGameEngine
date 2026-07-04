#pragma once

#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include "../Graphics/Renderer/Shader/Shader.hpp"
#include "../Graphics/Mesh/Mesh.hpp"

#include <vector>

namespace Core {
namespace Render {
    class Texture;
    class Shader;
}
namespace Graphics {
    class Mesh;
}

class Entity {
private:
    std::shared_ptr<Core::Graphics::Mesh> m_mesh;
    std::shared_ptr<Core::Render::Shader> m_shader;
    std::vector<std::shared_ptr<Core::Render::Texture>> m_textures;

public:
    Entity(std::shared_ptr<Core::Graphics::Mesh> mesh, std::shared_ptr<Core::Render::Shader> shader);
    ~Entity() = default;

    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    void addTexture(std::shared_ptr<Core::Render::Texture> texture);
    const std::vector<std::shared_ptr<Core::Render::Texture>>& getTextures() const;

    glm::mat4 getModel() const;
    std::shared_ptr<Core::Graphics::Mesh> getMesh();
    std::shared_ptr<Core::Render::Shader> getShader();
};
}
