#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include "../Shader/Shader.hpp"
#include <GL/glew.h>
#include <iostream>

namespace Core::Render {

Texture::Texture(const char* imagePath, const std::string& texType, unsigned int slot)
    : type(texType), slot(slot) {
    
    glGenTextures(1, &ID);

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(imagePath, &width, &height, &numChannels, 0);

    if (bytes) {
        GLenum format = GL_RGB;
        if (numChannels == 1)
            format = GL_RED;
        else if (numChannels == 3)
            format = GL_RGB;
        else if (numChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << imagePath << std::endl;
    }

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

Texture::Texture(Texture&& other) noexcept 
    : ID(other.ID), type(std::move(other.type)), slot(other.slot) {
    other.ID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteTextures(1, &ID);
        }
        ID = other.ID;
        type = std::move(other.type);
        slot = other.slot;
        other.ID = 0;
    }
    return *this;
}

void Texture::texUnit(const Shader& shader, const char* uniform, unsigned int slot) {
    shader.activate();
    glUniform1i(glGetUniformLocation(shader.ID, uniform), slot);
}

void Texture::bind() const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::del() {
    glDeleteTextures(1, &ID);
    ID = 0;
}

}
