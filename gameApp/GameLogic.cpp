#include "GameLogic.hpp"

#include <iostream>

#include "../core/Input/Input.hpp"


void GameLogic::onUpdate() {
    if (Core::Input::getKeyState(GLFW_KEY_ESCAPE) == Core::Input::KEY_PRESSED) {
        std::cout << "Escape pressed" << std::endl;
    }
}

void GameLogic::onRender() {

}
