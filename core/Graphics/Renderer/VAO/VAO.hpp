#pragma once

#include "../VBO/VBO.hpp"
#include <memory>

namespace Core::Render {
class VAO {
public:
    unsigned int ID = 0;

    VAO();
    ~VAO();

    // Prevent copying
    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    // Allow moving
    VAO(VAO&& other) noexcept;
    VAO& operator=(VAO&& other) noexcept;

    void linkVBO(
        std::shared_ptr<VBO> vbo,
        unsigned int layout,
        unsigned int size,
        unsigned int type,
        int stride,
        void* offset
    );

    void bind();
    void unbind();
    void del();
};
}
