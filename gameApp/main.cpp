#include "GameLogic.hpp"
#include "../core/Game/Game.hpp"

int main() {
    Core::GameSpecs gameSpecs;
    gameSpecs.title = "Flap Bird";
    gameSpecs.windowSpec.width = 1280;
    gameSpecs.windowSpec.height = 720;

    Core::Game game(gameSpecs);
    game.pushLogic<GameLogic>();
    game.run();
}
