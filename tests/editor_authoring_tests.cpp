#include "EditorMath.hpp"
#include "EditHistory.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <cmath>

void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
bool near(const glm::mat4& a,const glm::mat4& b){for(int i=0;i<4;++i)for(int j=0;j<4;++j)if(std::abs(a[i][j]-b[i][j])>0.001f)return false;return true;}
std::unique_ptr<Core::Graphics::Camera> camera(){return std::make_unique<Core::Graphics::Camera>(glm::vec3(0,0,5),glm::vec3(0,1,0),60,0.1f,100);}
int main(){
 try{
    auto view=glm::lookAt(glm::vec3(0,0,5),glm::vec3(0),glm::vec3(0,1,0));
    auto projection=glm::perspective(glm::radians(60.0f),2.0f,0.1f,100.0f);
    auto ray=EditorMath::viewportRay({500,250},{100,50},{800,400},view,projection);
    require(ray.has_value()&&std::abs(ray->direction.x)<1e-6f&&ray->direction.z<-0.99f,"Viewport origin/size must map center to the camera ray");
    auto scaled=EditorMath::viewportRay({1000,500},{200,100},{1600,800},view,projection);
    require(scaled&&glm::length(scaled->direction-ray->direction)<1e-6f,"DPI scaling must not change picking rays");
    require(!EditorMath::viewportRay({99,250},{100,50},{800,400},view,projection),"Mouse outside viewport must not generate a ray");
    require(!EditorMath::viewportRay({0,0},{0,0},{0,0},view,projection),"Zero-sized viewport must not generate a ray");
    Core::Scene scene(camera());
    auto* far=scene.createEntity(nullptr,nullptr);far->position={0,0,-2};far->setLocalAABB({glm::vec3(-0.5f),glm::vec3(0.5f)});
    auto* close=scene.createEntity(nullptr,nullptr);close->position={0,0,1};close->setLocalAABB(far->getLocalAABB());
    require(EditorMath::pick(scene,*ray)==close->id(),"Picking must choose the nearest object, not insertion order");
    close->position.x=8;require(EditorMath::pick(scene,*ray)==far->id(),"Picking must respect translated objects");
    require(!EditorMath::pick(scene,{{0,0,0},{0,0,0},100}),"Zero-direction rays must be rejected");
    require(!EditorMath::pick(scene,{{3,3,5},{0,0,-1},100}),"A ray parallel to and outside a slab must miss");
    require(!EditorMath::pick(scene,{{0,0,5},{0,0,1},100}),"Objects behind the ray must not be selected");
    require(!EditorMath::pick(scene,{{0,0,5},{0,0,-1},1}),"Objects beyond the far plane must not be selected");
    far->rotation={0,0,45};far->position={0,0,0};far->scale={4,0.2f,1};
    require(!EditorMath::pick(scene,{{1,-1,5},{0,0,-1},100}),"Picking must reject empty corners of a rotated world AABB");
    require(EditorMath::pick(scene,{{1,1,5},{0,0,-1},100})==far->id(),"Picking must hit rotated and non-uniformly scaled bounds");
    require(EditorMath::pick(scene,{{0,0,0},{0,0,-1},100})==far->id(),"Picking from inside an object must work");
    for(const auto angles:{glm::vec3(23,61,-17),glm::vec3(0,90,20),glm::vec3(-45,-89,120)}){
        Core::Entity source(nullptr,nullptr),target(nullptr,nullptr);
        source.position={-2,3,4};source.rotation=angles;source.scale={0.2f,3,1.5f};
        auto matrix=source.getModel();require(EditorMath::applyTransform(target,matrix),"Valid gizmo matrix was rejected");
        require(near(target.getModel(),matrix),"Matrix decomposition must match the engine's XYZ rotation order");
        require(!EditorMath::applyTransform(target,target.getModel()),"Unchanged matrices must not create edits");
    }
    Core::Entity target(nullptr,nullptr);auto original=target.getModel();
    auto invalid=original;invalid[0][0]=-1;require(!EditorMath::applyTransform(target,invalid),"Reflections must not introduce negative scale");
    invalid=original;invalid[0][0]=0.001f;require(!EditorMath::applyTransform(target,invalid),"Tiny scales must be rejected");
    invalid=original;invalid[1][0]=0.5f;require(!EditorMath::applyTransform(target,invalid),"Sheared transforms must be rejected");
    invalid=original;invalid[3][0]=std::numeric_limits<float>::infinity();require(!EditorMath::applyTransform(target,invalid),"Non-finite matrices must be rejected");
    invalid=original;invalid[0][3]=0.1f;require(!EditorMath::applyTransform(target,invalid),"Perspective matrices must be rejected");
    require(target.getModel()==original,"Rejected gizmo changes must preserve the original transform");
    EditHistory history;
    EditorFiles::Object object;object.id=42;object.name="Cube";
    EditorFiles::Document a{{object}},b=a;
    history.reset(a,42);require(!history.canUndo()&&!history.isDirty(a),"New history must be clean");
    require(!history.commit(a,0)&&history.size()==1,"Selection-only changes must not become undo actions");
    b.objects[0].name="Renamed";history.commit(b,42);
    require(history.canUndo()&&history.isDirty(b),"Renaming must be undoable and dirty");
    auto* state=history.undo();require(state&&state->document==a&&!state->selected,"Undo must restore document and selection");
    require(!history.isDirty(state->document),"Undo to saved content must clear dirty state");
    state=history.redo();require(state&&state->document==b&&state->selected==42,"Redo must restore document and selection");
    history.markSaved(b);auto c=b;c.objects[0].position={1,2,3};c.objects[0].mass=2;c.objects[0].gravity=true;
    history.commit(c,42);state=history.undo();require(state&&state->document==b&&!history.isDirty(state->document),"Saved checkpoint must survive undo/redo");
    auto d=b;d.objects.clear();history.commit(d,0);require(!history.canRedo(),"Editing after Undo must discard the redo branch");
    state=history.undo();require(state&&state->document==b,"Deleting an object must be undoable with all properties");
    history.reset(a,42);
    for(int i=0;i<140;++i){auto next=a;next.objects[0].position.x=static_cast<float>(i+1);history.commit(std::move(next),42);}
    require(history.size()==EditHistory::maxStates,"History snapshot count must be bounded");
    require(history.isDirty(a)==false,"Evicting old history must not lose the saved checkpoint");
    history.reset(a,42);
    for(int i=0;i<40;++i){auto next=a;next.objects[0].name=std::string(1024*1024,'x')+std::to_string(i);history.commit(std::move(next),42);}
    require(history.size()<40&&history.size()>1,"History memory budget must evict old snapshots");
    Core::Scene rebuilt(camera());auto* one=rebuilt.createEntity(nullptr,nullptr);
    auto high=scene.nextEntityId();rebuilt.preserveEntityIdsFrom(scene);
    require(rebuilt.createEntity(nullptr,nullptr)->id()>=high&&one->id()!=high,"Undo restore must preserve allocator high-water mark");
    std::cout<<"Viewport picking, XYZ gizmo transforms, and undo/redo checks passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
