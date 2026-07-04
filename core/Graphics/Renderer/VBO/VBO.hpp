#pragma once

namespace Core::Render {
class VBO {
public:
    unsigned int ID = 0;

    VBO(const void* data, long size);
    ~VBO();

    // Prevent copying
    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    // Allow moving
    VBO(VBO&& other) noexcept;
    VBO& operator=(VBO&& other) noexcept;

    void bind();
    void unbind();
    void del();
};
}
