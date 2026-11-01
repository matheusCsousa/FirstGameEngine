#include "PlaySession.hpp"
#include "AssetPaths.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void put(const std::filesystem::path& path,const std::string& value){std::ofstream file(path);file<<value;}
std::unique_ptr<Core::Graphics::Camera> camera(){return std::make_unique<Core::Graphics::Camera>(glm::vec3(0,0,5),glm::vec3(0,1,0),60,0.1f,100);}
int main(){
 try{
    auto root=std::filesystem::temp_directory_path()/("fersa-lua-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));std::filesystem::create_directories(root/"assets");
    put(root/"assets"/"rotate.lua",R"lua(return {
        on_start=function(self) self.count=0; counter=0; assert(io==nil and os==nil and require==nil and load==nil and debug==nil) end,
        on_update=function(self,dt) self.count=self.count+1; counter=counter+1; self.rotation.y=counter; self.position.x=self.position.x+dt*2 end
    })lua");
    Core::Scene source(camera());auto* a=source.createEntity(nullptr,nullptr);a->scriptAsset="assets/rotate.lua";a->name="First";
    auto* b=source.duplicateEntity(a->id());PlaySession play;play.start(source,camera(),root);
    require(play.diagnostics().empty(),"Valid script failed to start");play.advance(PlaySession::fixedStep);
    require(play.scene()->findEntity(a->id())->rotation.y==1&&play.scene()->findEntity(b->id())->rotation.y==1,"Scripts must have isolated globals and self state per object");
    require(play.scene()->findEntity(a->id())->position.x>0&&a->position.x==0,"Scripts must affect only the runtime copy");
    play.setPaused(true);play.advance(1);require(play.scene()->findEntity(a->id())->rotation.y==1,"Pause must pause scripts");play.step();require(play.scene()->findEntity(a->id())->rotation.y==2,"Step must tick scripts once");
    play.stop();play.start(source,camera(),root);play.advance(PlaySession::fixedStep);require(play.scene()->findEntity(a->id())->rotation.y==1,"Replay must reload fresh script state");play.stop();
    put(root/"assets"/"invalid.lua","return { on_update=function(self,dt) self.position.x=999; self.scale.x=0 end }");
    a->scriptAsset="assets/invalid.lua";play.start(source,camera(),root);play.advance(PlaySession::fixedStep);
    require(!play.diagnostics().empty()&&play.scene()->findEntity(a->id())->position.x==0,"Invalid script transforms must roll back and report an error");
    require(play.scene()->findEntity(b->id())->rotation.y==1,"One script error must not stop other objects");play.stop();
    put(root/"assets"/"loop.lua","return { on_update=function(self,dt) while true do end end }");a->scriptAsset="assets/loop.lua";
    play.start(source,camera(),root);play.advance(PlaySession::fixedStep);require(!play.diagnostics().empty(),"Infinite loops must hit the instruction budget");play.stop();
    put(root/"assets"/"memory.lua","return { on_update=function(self,dt) self.big=string.rep('x',8*1024*1024) end }");a->scriptAsset="assets/memory.lua";
    play.start(source,camera(),root);play.advance(PlaySession::fixedStep);require(!play.diagnostics().empty(),"Excess script memory must report an error without crashing");play.stop();
    put(root/"assets"/"bad.lua","return {");a->scriptAsset="assets/bad.lua";play.start(source,camera(),root);require(!play.diagnostics().empty(),"Syntax errors must be reported");play.stop();
    a->scriptAsset="assets/missing.lua";play.start(source,camera(),root);require(!play.diagnostics().empty(),"Missing script files must be reported");play.stop();
    a->scriptAsset="assets/../../escape.lua";play.start(source,camera(),root);require(!play.diagnostics().empty(),"Scripts outside project root must be rejected");play.stop();
    require(std::filesystem::equivalent(root.parent_path(),std::filesystem::temp_directory_path())&&root.filename().string().starts_with("fersa-lua-test-"),"Unexpected cleanup path");std::filesystem::remove_all(root);
    std::cout<<"Lua callbacks, isolation, reload, pause/step, and error budgets passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
