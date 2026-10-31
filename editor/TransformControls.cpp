#include "TransformControls.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>

bool EditorWidgets::editVector(const char* label, glm::vec3& value, float speed, bool positive) {
    const auto previous = value;
    // AlwaysClamp includes ClampZeroRange in ImGui 1.92: only scale has real bounds.
    const bool edited = ImGui::DragFloat3(label, &value.x, speed, positive ? 0.01f : 0.0f,
                        positive ? 1000.0f : 0.0f, "%.2f", positive ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None);
    if (edited) {
        for (int axis = 0; axis < 3; ++axis) {
            if (!std::isfinite(value[axis])) value[axis] = previous[axis];
            if (positive) value[axis] = std::clamp(value[axis], 0.01f, 1000.0f);
        }
    }
    return edited && value != previous;
}
