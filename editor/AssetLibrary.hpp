#pragma once
#include "AssetPaths.hpp"
#include "SceneDocument.hpp"
#include <map>
struct LoadedMesh { std::shared_ptr<Core::Graphics::Mesh> mesh; Core::Collision::AABB bounds; };
enum class AssetKind { Model,Texture,Script };
struct AssetEntry { std::string reference,name;AssetKind kind; };
class AssetLibrary {
    std::filesystem::path builtins,project;
    Core::AssetManager manager;
    std::map<std::string,LoadedMesh> meshes;
    std::map<std::string,std::shared_ptr<Core::Render::Texture>> textures;
    std::shared_ptr<Core::Render::Shader> surface;
    std::string uniqueReference(const std::string& filename) const;
public:
    AssetLibrary(std::filesystem::path builtins,std::filesystem::path project);
    const std::filesystem::path& root() const { return project; }
    const std::filesystem::path& builtinRoot() const { return builtins; }
    std::vector<AssetEntry> list() const;
    static AssetKind kind(const std::string& path);
    std::string importFile(const std::filesystem::path& source);
    std::string createScript();
    void saveScript(const std::string& reference,const std::string& source);
    std::string scriptSource(const std::string& reference) const;
    LoadedMesh mesh(const std::string& reference);
    std::shared_ptr<Core::Render::Texture> texture(const std::string& reference);
    std::shared_ptr<Core::Render::Shader> shader();
    void copyReferenced(const EditorFiles::Document& document,const std::filesystem::path& destination) const;
};
