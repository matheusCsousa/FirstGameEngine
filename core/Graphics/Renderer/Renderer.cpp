#include "Renderer.hpp"

#include <GL/glew.h>

namespace Core::Graphics {

    void Renderer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::Draw(
        const Core::Graphics::Mesh& mesh,
        const Core::Renderer::Shader& shader,
        const glm::mat4& model
    ) {
        shader.activate();

        shader.setMat4("model", model);

        mesh.draw();
    }
}
