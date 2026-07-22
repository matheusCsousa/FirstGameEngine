#include "AssetManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <limits>
#include <glm/glm.hpp>

namespace {
struct OBJVertex {
    int positionIndex;
    int texCoordIndex;
    int normalIndex;

    bool operator==(const OBJVertex& other) const {
        return positionIndex == other.positionIndex &&
               texCoordIndex == other.texCoordIndex &&
               normalIndex == other.normalIndex;
    }
};

struct OBJVertexHash {
    size_t operator()(const OBJVertex& v) const {
        return ((std::hash<int>()(v.positionIndex) ^
                (std::hash<int>()(v.texCoordIndex) << 1)) >> 1) ^
                (std::hash<int>()(v.normalIndex) << 1);
    }
};

void parseOBJVertex(const std::string& token, int& v, int& vt, int& vn) {
    v = 0; vt = 0; vn = 0;
    std::stringstream ss(token);
    std::string part;
    
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) v = std::stoi(part);
    }
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) vt = std::stoi(part);
    }
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) vn = std::stoi(part);
    }
}
}

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

std::shared_ptr<Core::Graphics::Mesh> AssetManager::loadMeshFromOBJ(
    const std::string& name, 
    const std::string& filepath,
    Core::Collision::AABB& outLocalAABB
) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return nullptr;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<glm::vec3> temp_normals;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<OBJVertex, unsigned int, OBJVertexHash> vertex_to_index;

    glm::vec3 minAABB(std::numeric_limits<float>::max());
    glm::vec3 maxAABB(-std::numeric_limits<float>::max());
    bool hasVertices = false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
            
            minAABB = glm::min(minAABB, pos);
            maxAABB = glm::max(maxAABB, pos);
            hasVertices = true;
        } else if (prefix == "vt") {
            glm::vec2 tex;
            ss >> tex.x >> tex.y;
            temp_texcoords.push_back(tex);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        } else if (prefix == "f") {
            std::vector<std::string> faceTokens;
            std::string faceToken;
            while (ss >> faceToken) {
                faceTokens.push_back(faceToken);
            }

            for (size_t i = 1; i < faceTokens.size() - 1; ++i) {
                std::string tokens[3] = { faceTokens[0], faceTokens[i], faceTokens[i+1] };

                for (int j = 0; j < 3; ++j) {
                    int vIdx, vtIdx, vnIdx;
                    parseOBJVertex(tokens[j], vIdx, vtIdx, vnIdx);

                    OBJVertex vertKey{ vIdx, vtIdx, vnIdx };

                    auto it = vertex_to_index.find(vertKey);
                    if (it != vertex_to_index.end()) {
                        indices.push_back(it->second);
                    } else {
                        unsigned int newIndex = static_cast<unsigned int>(vertices.size() / 11);
                        vertex_to_index[vertKey] = newIndex;
                        indices.push_back(newIndex);

                        glm::vec3 pos = (vIdx > 0 && vIdx <= (int)temp_positions.size()) ? temp_positions[vIdx - 1] : glm::vec3(0.0f);
                        glm::vec3 norm = (vnIdx > 0 && vnIdx <= (int)temp_normals.size()) ? temp_normals[vnIdx - 1] : glm::vec3(0.0f, 0.0f, 1.0f);
                        glm::vec2 tex = (vtIdx > 0 && vtIdx <= (int)temp_texcoords.size()) ? temp_texcoords[vtIdx - 1] : glm::vec2(0.0f);
                        tex.y = 1.0f - tex.y;

                        vertices.push_back(pos.x);
                        vertices.push_back(pos.y);
                        vertices.push_back(pos.z);
                        vertices.push_back(norm.x);
                        vertices.push_back(norm.y);
                        vertices.push_back(norm.z);
                        vertices.push_back(1.0f);
                        vertices.push_back(1.0f);
                        vertices.push_back(1.0f);
                        vertices.push_back(tex.x);
                        vertices.push_back(tex.y);
                    }
                }
            }
        }
    }

    if (hasVertices) {
        outLocalAABB = { minAABB, maxAABB };
    } else {
        outLocalAABB = { glm::vec3(0.0f), glm::vec3(0.0f) };
    }

    auto mesh = std::make_shared<Core::Graphics::Mesh>(
        vertices.data(), 
        vertices.size() * sizeof(float), 
        indices.data(), 
        indices.size() * sizeof(unsigned int)
    );
    m_meshes[name] = mesh;
    return mesh;
}

void AssetManager::clear() {
    m_shaders.clear();
    m_textures.clear();
    m_meshes.clear();
}

}
