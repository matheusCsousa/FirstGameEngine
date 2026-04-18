#pragma once

#include <vector>

namespace Core::Graphics {

    struct Vertex {
        float position[3];
        float color[3];
        float texCoord[2];
    };

    class Mesh {
    private:
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;

        unsigned int indexCount = 0;

    public:
        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices
        );

        ~Mesh();

        void draw() const;

        void bind() const;
        void unbind() const;
    };

}
