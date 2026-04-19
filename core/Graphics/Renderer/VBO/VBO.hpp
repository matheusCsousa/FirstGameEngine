#pragma once

#include <GL/glew.h>

namespace Core::Renderer {
class VBO {
public:
    GLuint ID;

    VBO(const void* data, GLsizeiptr size);

    void bind();
    void unbind();
    void del();
};
}
