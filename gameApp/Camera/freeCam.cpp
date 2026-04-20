#include "freeCam.hpp"

#include <GLFW/glfw3.h>
#include <glm/ext/vector_double2.hpp>
#include "../../core/Input/Input.hpp"

void FreeCam::update() {
   if (Core::Input::isKeyHeld(GLFW_KEY_W)) {
       position += front * speed * Core::Input::getDeltaTime();
   }
   if (Core::Input::isKeyHeld(GLFW_KEY_S)) {
       position -= front * speed * Core::Input::getDeltaTime();
   }
   if (Core::Input::isKeyHeld(GLFW_KEY_A)) {
       position -= right * speed * Core::Input::getDeltaTime();
   }
   if (Core::Input::isKeyHeld(GLFW_KEY_D)) {
       position += right * speed * Core::Input::getDeltaTime();
   }
   if (Core::Input::isKeyHeld(GLFW_KEY_E)) {
       position += worldUp * speed * Core::Input::getDeltaTime();
   }
   if (Core::Input::isKeyHeld(GLFW_KEY_Q)) {
       position -= worldUp * speed * Core::Input::getDeltaTime();
   }

    glm::dvec2 mouseChange = Core::Input::getMouseChange();
    yaw += mouseChange.x;
    pitch += mouseChange.y;
    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}
