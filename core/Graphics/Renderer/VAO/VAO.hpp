#pragma once

#include "../VBO/VBO.hpp"
#include <memory>

namespace Core::Render {
class VAO {
public:
    unsigned int ID;

    VAO();

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
