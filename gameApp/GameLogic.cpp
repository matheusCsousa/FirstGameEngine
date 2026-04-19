#include "GameLogic.hpp"

#include <iostream>

#include "../core/Input/Input.hpp"

GameLogic::GameLogic() {
    std::cout << "GameLogic started!" << std::endl;
}

void GameLogic::onUpdate() {
    if (Core::Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        std::cout << "Escape pressed!" << std::endl;
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
