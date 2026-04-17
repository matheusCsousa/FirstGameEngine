#include "Input.hpp"
#include <GLFW/glfw3.h>


namespace Core {

    Input::KeyState Input::keys[1024];

    double Input::lastX{};
    double Input::lastY{};
    double Input::xChange{};
    double Input::yChange{};
    GLfloat Input::deltaTime{};
    GLfloat Input::lastTime{};

    bool Input::mouseFirstMoved{true};


    void Input::initialize(GLFWwindow* window) {
        for (size_t i = 0; i < 1024; ++i) {
            keys[i] = Input::KEY_RELEASED;
        }
        glfwSetKeyCallback(window, handleKeyCallback);
        glfwSetCursorPosCallback(window, handleMouseCallback);
    }

    void Input::handleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)window;
        (void)scancode;
        (void)mods;
        if (action == Input::KEY_PRESSED) {
            keys[key] = Input::KEY_PRESSED;
        } else if (action == Input::KEY_RELEASED) {
            keys[key] = Input::KEY_RELEASED;
        }
    }


    Input::KeyState Input::getKeyState(int key) {
        return keys[key];
    }

    void Input::handleMouseCallback(GLFWwindow* window, double xpos, double ypos) {
        if (Input::getMouseMode(window) == GLFW_CURSOR_DISABLED) {
            if (Input::mouseFirstMoved) {
                Input::lastX = xpos;
                Input::lastY = ypos;
                Input::mouseFirstMoved = false;
            }
            Input::xChange = xpos - Input::lastX;
            Input::yChange = Input::lastY - ypos;
            Input::lastX = xpos;
            Input::lastY = ypos;
        }
        else {
            Input::mouseFirstMoved = true;
            Input::lastX = xpos;
            Input::lastY = ypos;
            Input::xChange = 0.0f;
            Input::yChange = 0.0f;
        }
    }

    void Input::setMouseMode(GLFWwindow* window, bool disabled) {
        if (disabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    glm::dvec2 Input::getMouseChange() {
        glm::dvec2 result(xChange, yChange);
        xChange = 0.0f;
        yChange = 0.0f;
        return result;
    }

    void Input::update() {
        xChange = 0.0f;
        yChange = 0.0f;

        for (size_t i = 0; i < 1024; ++i) {
            if (keys[i] == Input::KEY_PRESSED) {
                keys[i] = Input::KEY_HELD;
            }
        }

        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;
    }

    GLfloat Input::getDeltaTime() {
        return deltaTime;
    }

}
