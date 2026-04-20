#include "Camera.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Core::Graphics {

Camera::Camera(glm::vec3 position, glm::vec3 up, float fov, float nearPlane, float farPlane)
    : position(position),
        worldUp(up),
        fov(fov),
        nearPlane(nearPlane),
        farPlane(farPlane) {
}

glm::mat4 Camera::calcViewMatrix() {
    glm::vec3 front;

    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::calcProjectionMatrix(float aspectRatio) {
    return glm::perspective(
        glm::radians(fov),
        aspectRatio,
        nearPlane,
        farPlane
    );
}
}
