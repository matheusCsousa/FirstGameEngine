#pragma once
#include <array>
#include <string_view>
namespace BuiltinMeshes {
struct Shape { const char* name; const char* reference; };
inline constexpr std::array<Shape,7> shapes{{
    {"Cube", "models/cube.obj"}, {"Square (vertical)", "models/square.obj"},
    {"Plane (ground)", "models/plane.obj"}, {"Sphere", "models/sphere.obj"},
    {"Cylinder", "models/cylinder.obj"}, {"Cone", "models/cone.obj"},
    {"Capsule (bean)", "models/capsule.obj"}
}};
inline bool contains(std::string_view reference) {
    for (const auto& shape:shapes) if(reference==shape.reference)return true;
    return false;
}
}
