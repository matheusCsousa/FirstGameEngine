#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core::Graphics {
class Camera {
protected:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw{0.0f};
    float pitch{0.0f};

    float fov;
    float nearPlane;
    float farPlane;

public:
    Camera() {}
    Camera(glm::vec3 position, glm::vec3 up, float fov, float nearPlane, float farPlane);
    ~Camera() = default;

    virtual void update() {};
    glm::mat4 calcViewMatrix();
    glm::mat4 calcProjectionMatrix(float aspectRatio);
};
}
