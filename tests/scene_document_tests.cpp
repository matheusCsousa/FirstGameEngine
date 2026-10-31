#include "SceneDocument.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace EditorFiles;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejected(F function) { bool failed=false; try { function(); } catch (const std::exception&) { failed=true; } require(failed,"Invalid scene was accepted"); }
int main() {
    std::filesystem::path directory;
    try {
        Object a; a.id=42; a.name="Cube \"quoted\" / Ol\u00e1"; a.position={-3.5f,8,1}; a.rotation={-90,360,12.5f};
        a.scale={1.5f,0.2f,2}; a.mass=3.25f; a.gravity=true; a.velocity={1,2,3}; a.acceleration={4,5,6};
        Object b=a; b.id=9007199254740993ULL; b.name="Second";
        Document source{{a,b}};
        const auto text=encode(source);
        const auto loaded=decode(text);
        require(encode(loaded)==text,"Scene round trip changed authored data");
        require(loaded.objects[1].id==b.id,"Large IDs lost precision");
        require(decode(encode(Document{})).objects.empty(),"Empty scenes must round trip");
        auto json=nlohmann::json::parse(text);
        auto bad=json; bad["version"]=2; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][1]["id"]=42; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["id"]=-1; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["id"]=0; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["id"]=std::numeric_limits<std::uint64_t>::max(); rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["position"]={1,2}; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["scale"]={1,0,1}; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["position"]={1,1e100,1}; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["mesh"]="../unrelated.obj"; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0]["physics"]["mass"]=-1; rejected([&]{decode(bad.dump());});
        bad=json; bad["objects"][0].erase("name"); rejected([&]{decode(bad.dump());});
        rejected([&]{decode("{broken");});
        rejected([&]{decode(std::string(4*1024*1024+1,' '));});
        auto invalid=source; invalid.objects[0].position.x=std::numeric_limits<float>::infinity();
        rejected([&]{encode(invalid);});
        directory=std::filesystem::temp_directory_path()/("fersa-scene-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directory(directory);
        const auto path=directory/std::filesystem::path(u8"cena-\u00e1.fersa.json");
        write(path,source); require(encode(read(path))==text,"Saved file did not reopen correctly");
        rejected([&]{write(path,invalid);});
        require(encode(read(path))==text,"Failed save damaged the previous file");
        write(path,Document{}); require(read(path).objects.empty(),"Replacing existing file failed");
        rejected([&]{write(directory/"missing"/"scene.json",source);});
        rejected([&]{write(directory,source);});
        require(read(path).objects.empty(),"Unrelated failed write damaged a saved scene");
        require(std::distance(std::filesystem::directory_iterator(directory),std::filesystem::directory_iterator{})==1,
                "Failed save left temporary files behind");
        std::filesystem::remove(path); std::filesystem::remove(directory);
        std::cout<<"Scene document round trips and rejection checks passed\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
