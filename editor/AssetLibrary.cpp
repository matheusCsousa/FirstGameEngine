#include "BuiltinMeshes.hpp"
#include "AssetLibrary.hpp"
#include "ScriptRuntime.hpp"
#include "../core/AssetManager/ObjLoader.hpp"
#include <chrono>
#include <set>
#include <array>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

AssetLibrary::AssetLibrary(std::filesystem::path builtinPath,std::filesystem::path projectPath) : builtins(std::move(builtinPath)),project(std::filesystem::absolute(projectPath).lexically_normal()) {}
AssetKind AssetLibrary::kind(const std::string& name) {
    const auto ext=AssetPaths::extension(name);
    if(ext==".obj")return AssetKind::Model;
    if(ext==".lua")return AssetKind::Script;
    if(ext==".png"||ext==".jpg"||ext==".jpeg")return AssetKind::Texture;
    throw std::runtime_error("Supported imports: OBJ, PNG, JPG/JPEG, and Lua");
}
std::vector<AssetEntry> AssetLibrary::list() const {
    std::vector<AssetEntry> result;auto folder=project/"assets";if(!std::filesystem::exists(folder))return result;
    if(!AssetPaths::within(project,folder))throw std::runtime_error("Asset folder is outside the project");
    for(const auto& entry:std::filesystem::recursive_directory_iterator(folder))if(entry.is_regular_file()&&!entry.is_symlink()) {
        auto name=AssetPaths::text(entry.path().filename());
        try{auto type=kind(name);auto relative=AssetPaths::text(std::filesystem::relative(entry.path(),folder));result.push_back({"assets/"+relative,relative,type});}catch(const std::exception&){}
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.name<b.name;});return result;
}
std::string AssetLibrary::uniqueReference(const std::string& filename) const {
    std::string stem;for(unsigned char c:AssetPaths::text(AssetPaths::path(filename).stem())){if(stem.size()>=48)break;stem+=std::isalnum(c)?static_cast<char>(c):'_';}
    if(stem.empty())stem="asset";
    const auto suffix=AssetPaths::extension(filename);
    for(int i=0;i<100000;++i){auto name="assets/"+stem+(i?"_"+std::to_string(i):"")+suffix;if(!std::filesystem::exists(AssetPaths::resolve(project,name)))return name;}
    throw std::runtime_error("Too many assets with this name");
}
std::string AssetLibrary::importFile(const std::filesystem::path& source) {
    if(!std::filesystem::is_regular_file(source))throw std::runtime_error("Choose a regular file");
    const auto type=kind(AssetPaths::text(source));
    if(type==AssetKind::Model)Core::readOBJ(source);
    else if(type==AssetKind::Texture)Core::Render::Texture::validateFile(source);
    else {auto text=AssetPaths::readScript(source);auto error=ScriptRuntime::checkSyntax(text);if(!error.empty())throw std::runtime_error(error);}
    std::filesystem::create_directories(project/"assets");
    if(!AssetPaths::within(project,project/"assets"))throw std::runtime_error("Unsafe project asset folder");
    if(AssetPaths::within(project/"assets",source)) {
        auto relative=AssetPaths::text(std::filesystem::relative(source,project));
        if(AssetPaths::reference(relative))return relative;
    }
    auto reference=uniqueReference(AssetPaths::text(source.filename()));
    std::filesystem::copy_file(source,AssetPaths::resolve(project,reference),std::filesystem::copy_options::none);return reference;
}
std::string AssetLibrary::createScript() {
    std::filesystem::create_directories(project/"assets");
    auto reference=uniqueReference("Rotate.lua");
    saveScript(reference,R"lua(-- Each object gets its own self table and script state.
return {
    on_start = function(self)
        self.speed = 45 -- degrees per second
    end,
    on_update = function(self, dt)
        self.rotation.y = self.rotation.y + self.speed * dt
    end
}
)lua");return reference;
}
void AssetLibrary::saveScript(const std::string& reference,const std::string& source) {
    if(kind(reference)!=AssetKind::Script)throw std::runtime_error("Choose a Lua asset");
    auto error=ScriptRuntime::checkSyntax(source);if(!error.empty())throw std::runtime_error(error);
    const auto destination=AssetPaths::resolve(project,reference);
    auto temporary=destination;temporary+=".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    try {
        std::ofstream file(temporary,std::ios::binary);if(!file.write(source.data(),static_cast<std::streamsize>(source.size())))throw std::runtime_error("Cannot write script");
        file.close();if(!file)throw std::runtime_error("Cannot close script");
#ifdef _WIN32
        // Standard rename cannot replace an existing destination on Windows.
        if(!MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot replace script");
#else
        std::filesystem::rename(temporary,destination);
#endif
    }catch(...){std::error_code ignored;std::filesystem::remove(temporary,ignored);throw;}
}
std::string AssetLibrary::scriptSource(const std::string& reference) const {return AssetPaths::readScript(AssetPaths::resolve(project,reference));}
LoadedMesh AssetLibrary::mesh(const std::string& reference) {
    auto found=meshes.find(reference);if(found!=meshes.end())return found->second;
    const auto path=BuiltinMeshes::contains(reference)?builtins/AssetPaths::path(reference):AssetPaths::resolve(project,reference);
    if(AssetPaths::extension(reference)!=".obj")throw std::runtime_error("Unsupported mesh");
    LoadedMesh result;result.mesh=manager.loadMeshFromOBJ(reference,AssetPaths::text(path),result.bounds);meshes[reference]=result;return result;
}
std::shared_ptr<Core::Render::Shader> AssetLibrary::shader() {
    if(!surface)surface=manager.loadShader("surface",AssetPaths::text(builtins/"shader/shader.vert").c_str(),AssetPaths::text(builtins/"shader/shader.frag").c_str());return surface;
}
std::shared_ptr<Core::Render::Texture> AssetLibrary::texture(const std::string& reference) {
    auto found=textures.find(reference);if(found!=textures.end())return found->second;
    std::shared_ptr<Core::Render::Texture> result;
    if(reference=="builtin/white")result=std::make_shared<Core::Render::Texture>(std::array<unsigned char,4>{255,255,255,255});
    else {
        const auto path=reference=="builtin/brick"?builtins/"textures/brick.jpg":AssetPaths::resolve(project,reference);
        if(reference!="builtin/brick"&&kind(reference)!=AssetKind::Texture)throw std::runtime_error("Choose an image asset");
        result=manager.loadTexture(reference,AssetPaths::text(path).c_str(),"diffuse",0);
    }
    result->texUnit(*shader(),"tex0",0);textures[reference]=result;return result;
}
void AssetLibrary::copyReferenced(const EditorFiles::Document& document,const std::filesystem::path& destination) const {
    std::set<std::string> references;
    // Keep unassigned assets too: older undo snapshots may still reference them.
    for(const auto& entry:list())references.insert(entry.reference);
    for(const auto& object:document.objects)for(const auto& ref:{object.mesh,object.material,object.script})if(AssetPaths::reference(ref))references.insert(ref);
    for(const auto& ref:references) {
        auto from=AssetPaths::resolve(project,ref),to=AssetPaths::resolve(destination,ref);
        if(std::filesystem::exists(to)&&std::filesystem::equivalent(from,to))continue;
        if(std::filesystem::exists(to)) {
            std::ifstream a(from,std::ios::binary),b(to,std::ios::binary);
            if(!a||!b||std::filesystem::file_size(from)!=std::filesystem::file_size(to)||!std::equal(std::istreambuf_iterator<char>(a),std::istreambuf_iterator<char>(),std::istreambuf_iterator<char>(b)))throw std::runtime_error("Destination asset conflicts: "+ref);
        } else {std::filesystem::create_directories(to.parent_path());if(!AssetPaths::within(destination,to))throw std::runtime_error("Unsafe destination asset path");std::filesystem::copy_file(from,to);}
    }
}
