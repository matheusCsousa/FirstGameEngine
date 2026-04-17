#include <iostream>

#include "../core/Game/Game.hpp"
#include "GameLogic.hpp"


int main() {
    Core::GameSpecs gameSpecs;
    gameSpecs.title = "Flap Bird";
    gameSpecs.windowSpec.width = 1200;
    gameSpecs.windowSpec.height = 720;

    Core::Game game(gameSpecs);
    game.pushLogic<GameLogic>();
    game.run();

    std::cout << gameSpecs.title << " is running..." << std::endl;
}
