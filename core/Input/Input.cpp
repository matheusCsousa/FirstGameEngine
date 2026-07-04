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
        mouseButtons[i] = Input::KEY_RELEASED;
    }
}

void Input::onEvent(Event& event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
        if (e.getKeyCode() >= 0 && e.getKeyCode() < 1024) {
            keys[e.getKeyCode()] = Input::KEY_PRESSED;
        }
        return false;
    });

    dispatcher.dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) {
        if (e.getKeyCode() >= 0 && e.getKeyCode() < 1024) {
            keys[e.getKeyCode()] = Input::KEY_RELEASED;
        }
        return false;
    });

    dispatcher.dispatch<MouseMovedEvent>([](MouseMovedEvent& e) {
        if (Input::getMouseMode() == GLFW_CURSOR_DISABLED) {
            if (Input::mouseFirstMoved) {
                Input::lastX = e.getX();
                Input::lastY = e.getY();
                Input::mouseFirstMoved = false;
            }
            Input::xChange = e.getX() - Input::lastX;
            Input::yChange = Input::lastY - e.getY();
            Input::lastX = e.getX();
            Input::lastY = e.getY();
        } else {
            Input::mouseFirstMoved = true;
            Input::lastX = e.getX();
            Input::lastY = e.getY();
            Input::xChange = 0.0f;
            Input::yChange = 0.0f;
        }
        return false;
    });

    dispatcher.dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) {
        if (e.getMouseButton() >= 0 && e.getMouseButton() < 1024) {
            mouseButtons[e.getMouseButton()] = Input::KEY_PRESSED;
        }
        return false;
    });

    dispatcher.dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) {
        if (e.getMouseButton() >= 0 && e.getMouseButton() < 1024) {
            mouseButtons[e.getMouseButton()] = Input::KEY_RELEASED;
        }
        return false;
    });
}

bool Input::isKeyPressed(int key) {
    if (key < 0 || key >= 1024) return false;
    return keys[key] == Input::KEY_PRESSED;
}

bool Input::isKeyHeld(int key) {
    if (key < 0 || key >= 1024) return false;
    return keys[key] == Input::KEY_HELD;
}

Input::KeyState Input::getKeyState(int key) {
    if (key < 0 || key >= 1024) return Input::KEY_RELEASED;
    return keys[key];
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
