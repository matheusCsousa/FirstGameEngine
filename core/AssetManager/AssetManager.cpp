#include "AssetManager.hpp"
#include <iostream>

namespace Core {

std::shared_ptr<Core::Render::Shader> AssetManager::loadShader(const std::string& name, const char* vertexPath, const char* fragmentPath) {
    auto shader = std::make_shared<Core::Render::Shader>(vertexPath, fragmentPath);
    m_shaders[name] = shader;
    return shader;
}

std::shared_ptr<Core::Render::Shader> AssetManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Core::Render::Texture> AssetManager::loadTexture(const std::string& name, const char* imagePath, const std::string& type, unsigned int slot) {
    auto texture = std::make_shared<Core::Render::Texture>(imagePath, type, slot);
    m_textures[name] = texture;
    return texture;
}

std::shared_ptr<Core::Render::Texture> AssetManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Core::Graphics::Mesh> AssetManager::loadMesh(const std::string& name, const float* vertices, size_t vertexSize, const unsigned int* indices, size_t indexSize) {
    auto mesh = std::make_shared<Core::Graphics::Mesh>(vertices, vertexSize, indices, indexSize);
    m_meshes[name] = mesh;
    return mesh;
}

std::shared_ptr<Core::Graphics::Mesh> AssetManager::registerMesh(const std::string& name, const Core::Graphics::Mesh& mesh) {
    auto m = std::make_shared<Core::Graphics::Mesh>(mesh);
    m_meshes[name] = m;
    return m;
}

std::shared_ptr<Core::Graphics::Mesh> AssetManager::getMesh(const std::string& name) {
    auto it = m_meshes.find(name);
    if (it != m_meshes.end()) {
        return it->second;
    }
    return nullptr;
}

void AssetManager::clear() {
    m_shaders.clear();
    m_textures.clear();
    m_meshes.clear();
}

}
