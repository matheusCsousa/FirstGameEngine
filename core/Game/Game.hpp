#pragma once

#include <string>
#include <memory>

#include "../Window/Window.hpp"
#include "../Logic/Logic.hpp"


namespace Core {
struct GameSpecs {
    std::string title = "Game";
    Core::WindowSpecs windowSpec;
};

class Game {
private:
    GameSpecs m_specs;
    std::unique_ptr<Core::Logic> m_logic;
    std::shared_ptr<Core::Window> m_window;

    void onEvent(Event& event);
    bool onWindowClose(WindowCloseEvent& event);

public:
    Game(GameSpecs& specs);
    ~Game();
    void run();

    template<typename TLogic>
    void pushLogic() {
        m_logic = std::make_unique<TLogic>(m_window);
    };
};
}
