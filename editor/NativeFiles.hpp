#pragma once
#include <filesystem>
#include <vector>
struct GLFWwindow;
std::vector<std::filesystem::path> chooseAssetFiles(GLFWwindow* owner);
