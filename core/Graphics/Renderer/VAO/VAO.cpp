#include "VAO.hpp"
#include <GL/glew.h>
#include <memory>

namespace Core::Render {

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

VAO::~VAO() {
    if (ID != 0) {
        glDeleteVertexArrays(1, &ID);
    }
}

VAO::VAO(VAO&& other) noexcept : ID(other.ID) {
    other.ID = 0;
}

VAO& VAO::operator=(VAO&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteVertexArrays(1, &ID);
        }
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

void VAO::linkVBO(
    std::shared_ptr<VBO> vbo,
    unsigned int layout,
    unsigned int size,
    unsigned int type,
    int stride,
    void* offset
) {
    bind();
    vbo->bind();

    glVertexAttribPointer(
        layout,
        size,
        type,
        GL_FALSE,
        stride,
        offset
    );

    glEnableVertexAttribArray(layout);

    vbo->unbind();
    unbind();
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
