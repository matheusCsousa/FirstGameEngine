#pragma once

#include <string>
#include <array>
#include <filesystem>

namespace Core::Render {
class Shader;

class Texture {
public:
    unsigned int ID = 0;
    std::string type;
    unsigned int slot = 0;

    Texture(const char* imagePath, const std::string& texType, unsigned int slot);
    Texture(std::array<unsigned char,4> color);
    static void validateFile(const std::filesystem::path& path);
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
