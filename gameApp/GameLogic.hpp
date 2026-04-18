#pragma once

#include "../core/Logic/Logic.hpp"

class GameLogic : public Core::Logic {
public:
    GameLogic();
    ~GameLogic();

    void onUpdate() override;
    void onRender() override;

};
