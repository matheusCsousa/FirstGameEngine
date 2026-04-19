#include "Input.hpp"

#include <GLFW/glfw3.h>

namespace Core {

Input::KeyState Input::keys[1024];
Input::KeyState Input::mouseButtons[1024];

double Input::lastX{};
double Input::lastY{};
double Input::xChange{};
double Input::yChange{};
float Input::deltaTime{};
float Input::lastTime{};

bool Input::mouseFirstMoved{true};

GLFWwindow* Input::window{nullptr};

void Input::initialize(GLFWwindow* window) {
    Input::window = window;
    for (size_t i = 0; i < 1024; ++i) {
        keys[i] = Input::KEY_RELEASED;
    }
    glfwSetKeyCallback(Input::window, handleKeyCallback);
    glfwSetCursorPosCallback(Input::window, handleMouseCallback);
    glfwSetMouseButtonCallback(Input::window, handleMouseButtonCallback);
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

bool Input::isKeyPressed(int key) {
    return keys[key] == Input::KEY_PRESSED;
}

bool Input::isKeyHeld(int key) {
    return keys[key] == Input::KEY_HELD;
}

Input::KeyState Input::getKeyState(int key) {
    return keys[key];
}

void Input::handleMouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (Input::getMouseMode() == GLFW_CURSOR_DISABLED) {
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

void Input::handleMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == Input::KEY_PRESSED) {
        mouseButtons[button] = Input::KEY_PRESSED;
    }
    else if (action == Input::KEY_RELEASED) {
        mouseButtons[button] = Input::KEY_RELEASED;
    }
}

bool Input::isMouseButtonPressed(int button) {
    return mouseButtons[button] == Input::KEY_PRESSED;
}

bool Input::isMouseButtonHeld(int button) {
    return mouseButtons[button] == Input::KEY_HELD;
}

void Input::setMouseMode(bool disabled) {
    if (disabled) {
        glfwSetInputMode(Input::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(Input::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Input::toggleMouseMode() {
    if (Input::getMouseMode() == GLFW_CURSOR_DISABLED) {
        Input::setMouseMode(true);
    } else {
        Input::setMouseMode(false);
    }
}

int Input::getMouseMode() {
    return glfwGetInputMode(Input::window, GLFW_CURSOR);
};

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
        if (mouseButtons[i] == Input::KEY_PRESSED) {
            mouseButtons[i] = Input::KEY_HELD;
        }
    }


    float now = glfwGetTime();
    deltaTime = now - lastTime;
    lastTime = now;
}

float Input::getDeltaTime() {
    return deltaTime;
}
}
