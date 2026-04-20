#pragma once

namespace Core::Render {
class EBO {
public:
    unsigned int ID;

    EBO(const void* data, long size);
    ~EBO() = default;

    void bind();
    void unbind();
    void del();
};
}
