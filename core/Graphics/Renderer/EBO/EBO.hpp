#pragma once

namespace Core::Renderer {
class EBO {
public:
    unsigned int ID;

    EBO(const void* data, int size);

    void bind();
    void unbind();
    void del();
};
}
