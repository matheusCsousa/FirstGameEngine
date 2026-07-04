#include "EBO.hpp"
#include <GL/glew.h>

namespace Core::Render {

EBO::EBO(const void* data, long size) {
    glGenBuffers(1, &ID);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        size,
        data,
        GL_STATIC_DRAW
    );
}

EBO::~EBO() {
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
    }
}

EBO::EBO(EBO&& other) noexcept : ID(other.ID) {
    other.ID = 0;
}

EBO& EBO::operator=(EBO&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteBuffers(1, &ID);
        }
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

void EBO::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::del() {
    glDeleteBuffers(1, &ID);
}

}
