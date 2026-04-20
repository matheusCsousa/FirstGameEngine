#pragma once

#include <memory>

namespace Core::Render {
    struct VAO;
    struct VBO;
    struct EBO;
}

namespace Core::Graphics {

class Mesh {
private:
    std::shared_ptr<Core::Render::VAO> m_vao;
    std::shared_ptr<Core::Render::VBO> m_vbo;
    std::shared_ptr<Core::Render::EBO> m_ebo;

    int indexCount{};

public:
    Mesh() {}
    Mesh(
        const float* vertices,
        size_t vertexSize,
        const unsigned int* indices,
        size_t indexSize
    );

    ~Mesh() = default;

    void draw() const;

    void bind() const;
    void unbind() const;
};
}
