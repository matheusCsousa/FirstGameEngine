#include "Renderer.hpp"

#include <GL/glew.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Core::Graphics {

void Renderer::Clear()
{
    glClearColor(0.4f, 0.4f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(
    const Core::Graphics::Mesh& mesh,
    const Core::Render::Shader& shader,
    const glm::mat4& model
) {
    shader.activate();
    shader.setMat4("model", model);
    mesh.draw();
}
}
