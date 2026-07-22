#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "../Graphics/Renderer/Shader/Shader.hpp"
#include "../Graphics/Renderer/Texture/Texture.hpp"
#include "../Graphics/Mesh/Mesh.hpp"
#include <AABB.hpp>

namespace Core {

class AssetManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Core::Render::Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Core::Render::Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Core::Graphics::Mesh>> m_meshes;

public:
    AssetManager() = default;
    ~AssetManager() = default;

    std::shared_ptr<Core::Render::Shader> loadShader(const std::string& name, const char* vertexPath, const char* fragmentPath);
    std::shared_ptr<Core::Render::Shader> getShader(const std::string& name);

    std::shared_ptr<Core::Render::Texture> loadTexture(const std::string& name, const char* imagePath, const std::string& type, unsigned int slot);
    std::shared_ptr<Core::Render::Texture> getTexture(const std::string& name);

    std::shared_ptr<Core::Graphics::Mesh> loadMesh(const std::string& name, const float* vertices, size_t vertexSize, const unsigned int* indices, size_t indexSize);
    std::shared_ptr<Core::Graphics::Mesh> registerMesh(const std::string& name, const Core::Graphics::Mesh& mesh);
    std::shared_ptr<Core::Graphics::Mesh> getMesh(const std::string& name);
    std::shared_ptr<Core::Graphics::Mesh> loadMeshFromOBJ(const std::string& name, const std::string& filepath, Core::Collision::AABB& outLocalAABB);

    void clear();
};

}
