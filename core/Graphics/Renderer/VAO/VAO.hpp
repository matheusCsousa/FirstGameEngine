#pragma once

#include <GL/glew.h>
#include "../VBO/VBO.hpp"

namespace Core::Renderer {
class VAO {
public:
    GLuint ID;

    VAO();

    void linkVBO(
        VBO& vbo,
        GLuint layout,
        GLint size,
        GLenum type,
        GLsizei stride,
        void* offset
    );

    void bind();
    void unbind();
    void del();
};
}
