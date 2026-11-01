#include "AssetLibrary.hpp"
#include "ScriptRuntime.hpp"
#include "../core/AssetManager/ObjLoader.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>

void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejected(F function){bool failed=false;try{function();}catch(const std::exception&){failed=true;}require(failed,"Invalid asset was accepted");}
void put(const std::filesystem::path& path,const std::string& value){std::ofstream file(path,std::ios::binary);file<<value;}
int main(){
 try{
    const auto root=std::filesystem::temp_directory_path()/("fersa-assets-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root/"source");std::filesystem::create_directories(root/"project");
    const auto source=root/"source";const auto project=root/"project";
    const std::string quad="v -1 -1 0\nv 1 -1 0\nv 1 1 0\nv -1 1 0\nvt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\nf -4/-4 -3/-3 -2/-2 -1/-1\n";
    put(source/"quad.obj",quad);auto mesh=Core::readOBJ(source/"quad.obj");
    require(mesh.indices.size()==6&&mesh.vertices.size()==66,"OBJ polygons must triangulate");
    require(mesh.bounds.min==glm::vec3(-1,-1,0)&&mesh.bounds.max==glm::vec3(1,1,0),"OBJ bounds are incorrect");
    require(mesh.vertices[5]==1&&mesh.vertices[9]==0&&mesh.vertices[10]==0,"Generated normals and UV coordinates are incorrect");
    put(source/"bad.obj","v 0 0 0\nf\n");rejected([&]{Core::readOBJ(source/"bad.obj");});
    put(source/"bad.obj","v 0 0 0\nf 1 2 3\n");rejected([&]{Core::readOBJ(source/"bad.obj");});
    put(source/"bad.obj","v nan 0 0\nf 1 1 1\n");rejected([&]{Core::readOBJ(source/"bad.obj");});
    put(source/"bad.obj","v 0 0 0\nf 0 1 1\n");rejected([&]{Core::readOBJ(source/"bad.obj");});
    AssetLibrary library(std::filesystem::path(FERSA_TEST_ASSETS),project);
    const auto reference=library.importFile(source/"quad.obj");require(reference=="assets/quad.obj","Imported models need a portable reference");
    const auto second=library.importFile(source/"quad.obj");require(second!=reference,"Imports must not overwrite existing names");
    rejected([&]{library.importFile(source/"bad.obj");});
    require(library.list().size()==2,"Rejected imports must not create assets");
    const auto image=std::filesystem::path(FERSA_TEST_FIXTURES)/"rgb-odd.png";
    const auto texture=library.importFile(image);require(AssetLibrary::kind(texture)==AssetKind::Texture,"Image import failed");
    library.importFile(std::filesystem::path(FERSA_TEST_FIXTURES)/"gray-alpha.png");
    put(source/"bad.png","not an image");rejected([&]{library.importFile(source/"bad.png");});
    put(source/"bad.lua","return { broken");rejected([&]{library.importFile(source/"bad.lua");});
    auto script=library.createScript();require(ScriptRuntime::checkSyntax(library.scriptSource(script)).empty(),"Script template must compile");
    const auto saved=library.scriptSource(script);rejected([&]{library.saveScript(script,"return {");});
    require(library.scriptSource(script)==saved,"Invalid script save must preserve previous source");
    for(const auto& path:{"../escape.lua","assets/../../escape.lua","C:/escape.lua","assets\\escape.lua","/tmp/escape.lua"})rejected([&]{AssetPaths::resolve(project,path);});
    EditorFiles::Object object;object.id=1;object.name="Imported";object.mesh=reference;object.material=texture;object.script=script;object.tint={0.25f,0.5f,1};object.tiling={2,3};
    EditorFiles::Document document{{object}};require(EditorFiles::decode(EditorFiles::encode(document))==document,"Version 2 material/script data must round trip");
    EditorFiles::Object legacyObject;legacyObject.id=1;
    auto legacy=nlohmann::json::parse(EditorFiles::encode(EditorFiles::Document{{legacyObject}}));
    legacy["objects"][0]["id"]=1;legacy["version"]=1;
    for(const char* field:{"script","tint","tiling"})legacy["objects"][0].erase(field);
    require(EditorFiles::decode(legacy.dump()).objects[0].material=="builtin/brick","Version 1 scenes must remain readable");
    const auto destination=root/"copy";std::filesystem::create_directory(destination);
    library.copyReferenced(document,destination);
    require(std::filesystem::exists(AssetPaths::resolve(destination,reference))&&std::filesystem::exists(AssetPaths::resolve(destination,texture))&&std::filesystem::exists(AssetPaths::resolve(destination,script)),"Save As must copy all referenced assets");
    require(std::filesystem::exists(AssetPaths::resolve(destination,second)),"Save As must keep unassigned assets needed by undo history");
    library.copyReferenced(document,destination);put(AssetPaths::resolve(destination,script),"different");
    rejected([&]{library.copyReferenced(document,destination);});require(AssetPaths::readScript(AssetPaths::resolve(destination,script))=="different","Copy conflicts must preserve destination data");
    std::error_code linkError;std::filesystem::create_directory_symlink(source,project/"assets"/"outside",linkError);
    if(!linkError)rejected([&]{AssetPaths::resolve(project,"assets/outside/quad.obj");});
    // All files beneath this unique test-created temporary root belong to this test.
    require(root.parent_path()==std::filesystem::temp_directory_path()&&root.filename().string().starts_with("fersa-assets-test-"),"Unexpected cleanup path");
    std::filesystem::remove_all(root);
    std::cout<<"Import validation, portable assets, textures, scripts, and migration checks passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
