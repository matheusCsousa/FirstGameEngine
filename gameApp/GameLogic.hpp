#pragma once

#include "../core/Logic/Logic.hpp"
#include <memory>
#include <vector>

namespace Core {
    struct Window;
}


class GameLogic : public Core::Logic {
private:
    struct object;
    std::vector<object> m_objects;
    std::shared_ptr<Core::Window> m_window;


public:
    GameLogic(std::shared_ptr<Core::Window> window);
    ~GameLogic();

    void onUpdate() override;
    void onRender() override;
};
