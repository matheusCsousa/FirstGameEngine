#pragma once

#include <GL/glew.h>

namespace Core::Renderer {
class EBO {
public:
    GLuint ID;

    EBO(const void* data, GLsizeiptr size);

    void bind();
    void unbind();
    void del();
};
}
