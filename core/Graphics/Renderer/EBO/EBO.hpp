#pragma once

namespace Core::Render {
class EBO {
public:
    unsigned int ID = 0;

    EBO(const void* data, long size);
    ~EBO();

    // Prevent copying
    EBO(const EBO&) = delete;
    EBO& operator=(const EBO&) = delete;

    // Allow moving
    EBO(EBO&& other) noexcept;
    EBO& operator=(EBO&& other) noexcept;

    void bind();
    void unbind();
    void del();
};
}
