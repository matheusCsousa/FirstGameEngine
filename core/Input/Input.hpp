#pragma once

#include <glm/vec2.hpp>
#include "../Event/Event.hpp"

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

    static void onEvent(Event& event);

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
