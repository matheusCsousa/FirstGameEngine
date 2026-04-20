#pragma once

#include "../Mesh/Mesh.hpp"
#include "Shader/Shader.hpp"

#include <glm/mat4x4.hpp>

namespace Core::Graphics {
class Renderer {
public:
    static void Draw(
        const Core::Graphics::Mesh& mesh,
        const Core::Render::Shader& shader,
        const glm::mat4& model
    );

    static void Clear();
};
}
