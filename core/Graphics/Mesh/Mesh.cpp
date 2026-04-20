#include "Mesh.hpp"
#include <GL/glew.h>
#include <memory>

#include "../Renderer/VAO/VAO.hpp"
#include "../Renderer/VBO/VBO.hpp"
#include "../Renderer/EBO/EBO.hpp"

namespace Core::Graphics {

Mesh::Mesh(
    const float* vertices,
    size_t vertexSize,
    const unsigned int* indices,
    size_t indexSize
)
    : indexCount(indexSize / sizeof(unsigned int)),
      m_vao(std::make_shared<Core::Render::VAO>()),
      m_vbo(std::make_shared<Core::Render::VBO>(vertices, vertexSize)),
      m_ebo(std::make_shared<Core::Render::EBO>(indices, indexSize))
{
    m_vao->bind();
    m_ebo->bind();

    GLsizei stride = 8 * sizeof(float);

    m_vao->linkVBO(m_vbo, 0, 3, GL_FLOAT, stride, (void*)0);
    m_vao->linkVBO(m_vbo, 1, 3, GL_FLOAT, stride, (void*)(3 * sizeof(float)));
    m_vao->linkVBO(m_vbo, 2, 2, GL_FLOAT, stride, (void*)(6 * sizeof(float)));

    m_vao->unbind();
}

void Mesh::bind() const {
    m_vao->bind();
}

void Mesh::unbind() const {
    glBindVertexArray(0);
}

void Mesh::draw() const {
    m_vao->bind();
    glDrawElements(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        0
    );
}

}
