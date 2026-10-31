#pragma once
#include <glm/glm.hpp>
namespace EditorWidgets {
bool editVector(const char* label, glm::vec3& value, float speed, bool positive = false);
}
