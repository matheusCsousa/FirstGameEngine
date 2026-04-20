#pragma once

#include <glm/glm.hpp>
#include "../Graphics/Camera/Camera.hpp"

class FreeCam : public Core::Graphics::Camera {
private:
    float speed = 5.0f;
    float sensitivity = 0.6f;

public:
    using Core::Graphics::Camera::Camera;
    void update() override;
};
