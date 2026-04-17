#pragma once

#include "../core/Logic/Logic.hpp"

class GameLogic : public Core::Logic {
public:
    void onUpdate() override;
    void onRender() override;

};
