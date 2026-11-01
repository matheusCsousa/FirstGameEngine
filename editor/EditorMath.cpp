#define GLM_ENABLE_EXPERIMENTAL
#include "EditorMath.hpp"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cmath>
#include <algorithm>

namespace EditorMath {
namespace {
bool finite(glm::vec3 v) {return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
bool finite(const glm::mat4& m) {for(int i=0;i<4;++i)for(int j=0;j<4;++j)if(!std::isfinite(m[i][j]))return false;return true;}
}
std::optional<Ray> viewportRay(glm::vec2 mouse,glm::vec2 origin,glm::vec2 size,const glm::mat4& view,const glm::mat4& projection) {
    if(size.x<=0 || size.y<=0 || !std::isfinite(size.x) || !std::isfinite(size.y))return std::nullopt;
    auto uv=(mouse-origin)/size;
    if(!std::isfinite(uv.x)||!std::isfinite(uv.y)||uv.x<0||uv.x>1||uv.y<0||uv.y>1)return std::nullopt;
    auto inverse=glm::inverse(projection*view);
    auto near=inverse*glm::vec4(uv.x*2-1,1-uv.y*2,-1,1);
    auto far=inverse*glm::vec4(uv.x*2-1,1-uv.y*2,1,1);
    if(std::abs(near.w)<1e-8f||std::abs(far.w)<1e-8f)return std::nullopt;
    auto start=glm::vec3(near)/near.w;auto end=glm::vec3(far)/far.w;
    float length=glm::length(end-start);
    if(!finite(start)||!finite(end)||!std::isfinite(length)||length<1e-6f)return std::nullopt;
    return Ray{start,(end-start)/length,length};
}
Core::EntityId pick(const Core::Scene& scene,const Ray& ray) {
    Core::EntityId result=0;float closest=ray.length;
    if(!finite(ray.origin)||!finite(ray.direction)||glm::length(ray.direction)<1e-8f||!std::isfinite(closest)||closest<=0)return 0;
    for(const auto& entity:scene.getEntities()) {
        const auto model=entity->getModel();
        if(!finite(model)||std::abs(glm::determinant(model))<1e-10f)continue;
        const auto inverse=glm::inverse(model);
        auto origin=glm::vec3(inverse*glm::vec4(ray.origin,1));
        auto direction=glm::vec3(inverse*glm::vec4(ray.direction,0)); // Do not normalize: preserve world-space ray distance.
        const auto bounds=entity->getLocalAABB();
        float enter=0,exit=closest;bool hit=true;
        for(int axis=0;axis<3;++axis) {
            if(std::abs(direction[axis])<1e-8f) {if(origin[axis]<bounds.min[axis]||origin[axis]>bounds.max[axis])hit=false;}
            else {
                float a=(bounds.min[axis]-origin[axis])/direction[axis],b=(bounds.max[axis]-origin[axis])/direction[axis];
                if(a>b)std::swap(a,b); enter=std::max(enter,a);exit=std::min(exit,b);
                if(enter>exit)hit=false;
            }
        }
        if(hit && enter<closest) {closest=enter;result=entity->id();}
    }
    return result;
}
bool applyTransform(Core::Entity& entity,const glm::mat4& model) {
    if(!finite(model)||std::abs(model[0][3])>1e-6f||std::abs(model[1][3])>1e-6f||std::abs(model[2][3])>1e-6f||std::abs(model[3][3]-1)>1e-6f)return false;
    glm::vec3 position=model[3],scale{glm::length(glm::vec3(model[0])),glm::length(glm::vec3(model[1])),glm::length(glm::vec3(model[2]))};
    for(int i=0;i<3;++i)if(scale[i]<0.01f-1e-5f||scale[i]>1000+1e-3f)return false;
    glm::mat4 rotation(1);
    for(int i=0;i<3;++i)rotation[i]=glm::vec4(glm::vec3(model[i])/scale[i],0);
    if(glm::determinant(rotation)<0 || std::abs(glm::dot(glm::vec3(rotation[0]),glm::vec3(rotation[1])))>0.001f ||
       std::abs(glm::dot(glm::vec3(rotation[0]),glm::vec3(rotation[2])))>0.001f ||
       std::abs(glm::dot(glm::vec3(rotation[1]),glm::vec3(rotation[2])))>0.001f)return false;
    float x,y,z; glm::extractEulerAngleXYZ(rotation,x,y,z);
    auto angles=glm::degrees(glm::vec3(x,y,z));
    if(!finite(position)||!finite(angles)||!finite(scale))return false;
    const auto previous=entity.getModel(); float difference=0;
    for(int i=0;i<4;++i)for(int j=0;j<4;++j)difference=std::max(difference,std::abs(previous[i][j]-model[i][j]));
    if(difference<1e-5f)return false;
    entity.position=position;entity.rotation=angles;entity.scale=glm::clamp(scale,glm::vec3(0.01f),glm::vec3(1000));return true;
}
}
