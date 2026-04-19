#pragma once

#include <glm/vec2.hpp>

struct GLFWwindow;

namespace Core {
class Input {
public:
    enum KeyState {
        KEY_RELEASED,
        KEY_PRESSED,
        KEY_HELD
    };
    static void initialize(GLFWwindow* window);

    static bool isMouseButtonPressed(int button);
    static bool isMouseButtonHeld(int button);
    static bool isKeyPressed(int key);
    static bool isKeyHeld(int key);
    static KeyState getKeyState(int key);

    static void update();

    static void handleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void handleMouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void handleMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void setMouseMode(bool disabled);
    static void toggleMouseMode();
    static int getMouseMode();

    static glm::dvec2 getMouseChange();

    static float getDeltaTime();

private:
    static GLFWwindow* window;
    static KeyState keys[1024];
    static KeyState mouseButtons[1024];
    static double lastX;
    static double lastY;
    static double xChange;
    static double yChange;
    static float deltaTime;
    static float lastTime;
    static bool mouseFirstMoved;

};
}
