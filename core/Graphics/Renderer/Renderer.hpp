#pragma once

#include "../Mesh/Mesh.hpp"
#include "Shader/Shader.hpp"

#include <glm/glm.hpp>

namespace Core::Graphics {
class Renderer {
public:
    void Draw(
        const Core::Graphics::Mesh& mesh,
        const Core::Renderer::Shader& shader,
        const glm::mat4& model
    );

    void Clear();
};
}
