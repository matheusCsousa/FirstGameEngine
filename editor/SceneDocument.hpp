#pragma once
#include "../core/Scene/Scene.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace EditorFiles {
// Version 2 stores portable project asset references. Version 1 remains readable.
struct Object {
    Core::EntityId id = 0;
    std::string name;
    std::string mesh = "models/cube.obj";
    std::string material = "builtin/brick";
    std::string script;
    glm::vec3 tint{1,1,1};
    glm::vec2 tiling{1,1};
    glm::vec3 position{}, rotation{}, scale{1,1,1}, velocity{}, acceleration{};
    float mass = 0;
    bool gravity = false;
    bool operator==(const Object&) const = default;
};
struct Document { std::vector<Object> objects; bool operator==(const Document&) const = default; };
Document capture(const Core::Scene& scene);
std::string encode(const Document& document);
Document decode(const std::string& text);
Document read(const std::filesystem::path& path);
void write(const std::filesystem::path& path, const Document& document);
}
