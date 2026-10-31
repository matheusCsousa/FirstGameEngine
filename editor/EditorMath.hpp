#pragma once
#include "../core/Scene/Scene.hpp"
#include <optional>
namespace EditorMath {
struct Ray { glm::vec3 origin,direction; float length; };
std::optional<Ray> viewportRay(glm::vec2 mouse,glm::vec2 origin,glm::vec2 size,const glm::mat4& view,const glm::mat4& projection);
Core::EntityId pick(const Core::Scene& scene,const Ray& ray);
bool applyTransform(Core::Entity& entity,const glm::mat4& model);
}
