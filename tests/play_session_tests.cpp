#include "PlaySession.hpp"
#include "SceneDocument.hpp"
#include "../core/Physics/Physics.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
std::unique_ptr<Core::Graphics::Camera> camera() { return std::make_unique<Core::Graphics::Camera>(glm::vec3(0,0,5),glm::vec3(0,1,0),60,0.1f,100); }
int main() {
    try {
        Core::Scene authored(camera());
        auto* cube=authored.createEntity(nullptr,nullptr,42);
        cube->name="Falling cube"; cube->position={0,5,0}; cube->rotation={10,20,30}; cube->scale={1,2,1};
        cube->mass=2; cube->useGravity=true; cube->velocity={1,0,0};
        cube->setLocalAABB({{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,0.5f}});
        auto* removed=authored.createEntity(nullptr,nullptr,100);
        authored.removeEntity(removed->id());
        const auto original=EditorFiles::encode(EditorFiles::capture(authored));
        PlaySession play;
        play.start(authored,camera());
        require(play.isPlaying() && !play.isPaused(),"Play did not start");
        auto* runtime=play.scene()->findEntity(42);
        require(runtime && runtime!=cube && runtime->name==cube->name,"Play must clone object identity and independent state");
        require(runtime->getLocalAABB().min==cube->getLocalAABB().min && runtime->rotation==cube->rotation && runtime->scale==cube->scale,"Clone lost authored properties");
        require(play.scene()->createEntity(nullptr,nullptr)->id()>100,"Clone must preserve deleted-ID allocator history");
        play.advance(1.0/60);
        require(runtime->position.y<5 && runtime->position.x>0,"Play must simulate gravity and initial velocity");
        require(EditorFiles::encode(EditorFiles::capture(authored))==original,"Simulation changed authored data");
        const auto count=play.stepCount();
        play.start(authored,camera()); require(play.stepCount()==count,"Repeated Play must not reset a running session");
        play.setPaused(true); auto y=runtime->position.y;
        play.advance(10); require(runtime->position.y==y && play.stepCount()==count,"Pause must stop simulation");
        play.step(); require(play.stepCount()==count+1 && runtime->position.y<y,"Step must advance exactly one fixed tick");
        play.setPaused(false); const auto resumed=play.stepCount();
        play.advance(PlaySession::fixedStep); require(play.stepCount()==resumed+1,"Resume must not catch up paused time");
        play.step(); require(play.stepCount()==resumed+1,"Step must do nothing while running");
        play.advance(60); require(play.stepCount()==resumed+1+PlaySession::maxStepsPerFrame,"Long frames must have bounded catch-up");
        const auto bounded=play.stepCount();
        play.advance(-1); play.advance(std::numeric_limits<double>::quiet_NaN()); play.advance(std::numeric_limits<double>::infinity());
        require(play.stepCount()==bounded,"Invalid frame time must not advance simulation");
        play.stop(); play.stop();
        require(!play.isPlaying() && !play.scene(),"Stop must release runtime state");
        require(EditorFiles::encode(EditorFiles::capture(authored))==original,"Stop changed authored transforms or physics state");
        play.start(authored,camera()); require(play.scene()->findEntity(42)->position==cube->position,"Replay must start from authored state"); play.stop();
        PlaySession sixty,oneTwenty;
        sixty.start(authored,camera()); oneTwenty.start(authored,camera());
        for(int i=0;i<60;++i) sixty.advance(1.0/60);
        for(int i=0;i<120;++i) oneTwenty.advance(1.0/120);
        require(sixty.stepCount()==120 && oneTwenty.stepCount()==120,"Fixed timestep count varies by render rate");
        require(sixty.scene()->findEntity(42)->position==oneTwenty.scene()->findEntity(42)->position,"Physics differs across render rates");
        Core::Scene collision(camera());
        auto* a=collision.createEntity(nullptr,nullptr); a->mass=1; a->setLocalAABB({{0,0,0},{2,2,6}});
        auto* b=collision.createEntity(nullptr,nullptr); b->setLocalAABB({{1,1,1},{3,3,7}});
        Core::Physics::PhysicsEngine::update(1.0f/120,collision.getEntities());
        require(a->position.x==-1 && a->position.y==0 && a->position.z==0,"Equal-depth collision must choose a minimum penetration axis");
        Core::Scene landing(camera());
        auto* falling=landing.createEntity(nullptr,nullptr); falling->position={0,4,0}; falling->mass=1; falling->useGravity=true;
        falling->setLocalAABB({{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,0.5f}});
        auto* floor=landing.createEntity(nullptr,nullptr); floor->setLocalAABB({{-5,-0.2f,-5},{5,0,5}});
        PlaySession drop; drop.start(landing,camera());
        for(int i=0;i<600;++i) drop.advance(PlaySession::fixedStep);
        const auto finalY=drop.scene()->findEntity(falling->id())->position.y;
        require(std::isfinite(finalY) && finalY>=0.49f && finalY<0.6f,"Falling body must land on static ground");
        std::cout<<"Play isolation, fixed stepping, pause/resume, and collision checks passed\n"; return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
