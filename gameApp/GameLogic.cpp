#include "GameLogic.hpp"

#include <iostream>

#include "../core/Input/Input.hpp"
#include "../core/Graphics/Renderer/Renderer.hpp"
#include "../core/Graphics/Mesh/Mesh.hpp"
#include "../core/Graphics/Renderer/Shader/Shader.hpp"

GameLogic::GameLogic(){
    std::cout << "GameLogic started!" << std::endl;
}

GameLogic::~GameLogic() {
    std::cout << "GameLogic stopped!" << std::endl;
}

void GameLogic::onUpdate() {
    if (Core::Input::getKeyState(GLFW_KEY_ESCAPE) == Core::Input::KEY_PRESSED) {
        std::cout << "Escape pressed!" << std::endl;
    }
    if (Core::Input::getKeyState(GLFW_KEY_F) == Core::Input::KEY_PRESSED) {
        std::cout << "F pressed!" << std::endl;
    }
}

void GameLogic::onRender() {

}
