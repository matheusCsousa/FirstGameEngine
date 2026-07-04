#pragma once

#include <string>

namespace Core::Render {
class Shader;

class Texture {
public:
    unsigned int ID = 0;
    std::string type;
    unsigned int slot = 0;

    Texture(const char* imagePath, const std::string& texType, unsigned int slot);
    ~Texture();

    // Prevent copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Allow moving
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void texUnit(const Shader& shader, const char* uniform, unsigned int slot);
    void bind() const;
    void unbind() const;
    void del();
};
}
