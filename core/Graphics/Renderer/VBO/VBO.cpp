#include "VBO.hpp"

#include <GL/glew.h>
namespace Core::Render {

VBO::VBO(const void* data, long size) {
    glGenBuffers(1, &ID);

    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VBO::~VBO() {
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
    }
}

VBO::VBO(VBO&& other) noexcept : ID(other.ID) {
    other.ID = 0;
}

VBO& VBO::operator=(VBO&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteBuffers(1, &ID);
        }
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

void VBO::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::del() {
    glDeleteBuffers(1, &ID);
}

}
