#include "VBO.hpp"

namespace Core::Renderer {

    VBO::VBO(const void* data, GLsizeiptr size) {
        glGenBuffers(1, &ID);

        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
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
