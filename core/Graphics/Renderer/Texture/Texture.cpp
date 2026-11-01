#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include "../Shader/Shader.hpp"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <memory>

namespace Core::Render {

namespace {
std::vector<unsigned char> imageBytes(const std::filesystem::path& path) {
    const auto size=std::filesystem::file_size(path);
    if(size==0||size>64*1024*1024)throw std::runtime_error("Image must be between 1 byte and 64 MiB");
    std::ifstream file(path,std::ios::binary);std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    if(!file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(size)))throw std::runtime_error("Cannot read image");
    int width,height,channels;
    if(!stbi_info_from_memory(bytes.data(),static_cast<int>(bytes.size()),&width,&height,&channels)||width<=0||height<=0||width>8192||height>8192||static_cast<long long>(width)*height>16*1024*1024)
        throw std::runtime_error("Invalid image or dimensions exceed 8192 / 16 megapixels");
    return bytes;
}
void upload(unsigned int& id,int width,int height,const unsigned char* pixels) {
    GLint limit;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);if(width>limit||height>limit)throw std::runtime_error("Image exceeds this GPU texture limit");
    GLint active,binding,alignment;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
    glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);glGenerateMipmap(GL_TEXTURE_2D);
    GLint uploadedWidth;glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&uploadedWidth);
    glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);glBindTexture(GL_TEXTURE_2D,static_cast<GLuint>(binding));glActiveTexture(static_cast<GLenum>(active));
    if(uploadedWidth!=width){glDeleteTextures(1,&id);id=0;throw std::runtime_error("GPU texture allocation failed");}
}
}
void Texture::validateFile(const std::filesystem::path& path) {
    const auto bytes=imageBytes(path);int w,h,c;
    std::unique_ptr<unsigned char,decltype(&stbi_image_free)> pixels(stbi_load_from_memory(bytes.data(),static_cast<int>(bytes.size()),&w,&h,&c,4),stbi_image_free);
    if(!pixels)throw std::runtime_error("Image decoding failed");
}
Texture::Texture(const char* imagePath,const std::string& texType,unsigned int textureSlot) : type(texType),slot(textureSlot) {
    std::string name(imagePath);const auto bytes=imageBytes(std::filesystem::path(std::u8string(name.begin(),name.end())));
    int w,h,c;stbi_set_flip_vertically_on_load(true);
    std::unique_ptr<unsigned char,decltype(&stbi_image_free)> pixels(stbi_load_from_memory(bytes.data(),static_cast<int>(bytes.size()),&w,&h,&c,4),stbi_image_free);
    if(!pixels)throw std::runtime_error("Image decoding failed: "+name);
    upload(ID,w,h,pixels.get());
}
Texture::Texture(std::array<unsigned char,4> color) : type("diffuse"),slot(0) { upload(ID,1,1,color.data()); }

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
