#pragma once

namespace Core::Render {
class VBO {
public:
    unsigned int ID;

    VBO(const void* data, long size);

    void bind();
    void unbind();
    void del();
};
}
