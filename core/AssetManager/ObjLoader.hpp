#pragma once
#include <filesystem>
#include <vector>
#include <AABB.hpp>
namespace Core {
struct ObjData { std::vector<float> vertices; std::vector<unsigned int> indices; Collision::AABB bounds; };
ObjData readOBJ(const std::filesystem::path& path);
}
