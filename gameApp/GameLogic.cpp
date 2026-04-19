#include "GameLogic.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "../core/Input/Input.hpp"
#include "../core/Window/Window.hpp"

GameLogic::GameLogic(std::shared_ptr<Core::Window> window)
    : m_window(window) {
    std::cout << "GameLogic started!" << std::endl;
}

void GameLogic::onUpdate() {
    if (Core::Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        std::cout << "Escape pressed!" << std::endl;
        m_window->requestClose();
    }
    if (Core::Input::isKeyPressed(GLFW_KEY_F)) {
        std::cout << "F pressed!" << std::endl;
        Core::Input::toggleMouseMode();
    }
    if (Core::Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        std::cout << "Left mouse button pressed!" << std::endl;
    }
}

void GameLogic::onRender() {

}

GameLogic::~GameLogic() {
    std::cout << "GameLogic stopped!" << std::endl;
}
