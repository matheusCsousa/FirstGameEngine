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
        std::unique_ptr<Logic> m_logic;
        std::unique_ptr<Window> m_window;


    public:
        Game(GameSpecs& specs);
        ~Game();
        void run();

        template<typename TLogic>
        void pushLogic() {
            m_logic = std::make_unique<TLogic>();
        };
    };
}
