#include "VAO.hpp"

namespace Core::Renderer {

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::linkVBO(
    VBO& vbo,
    GLuint layout,
    GLint size,
    GLenum type,
    GLsizei stride,
    void* offset
) {
    vbo.bind();

    glVertexAttribPointer(
        layout,
        size,
        type,
        GL_FALSE,
        stride,
        offset
    );

    glEnableVertexAttribArray(layout);

    vbo.unbind();
}

void VAO::bind() {
    glBindVertexArray(ID);
}

void VAO::unbind() {
    glBindVertexArray(0);
}

void VAO::del() {
    glDeleteVertexArrays(1, &ID);
}

}
