#include "Game.hpp"

#include <GL/glew.h>

#include "../Window/Window.hpp"

namespace Core {

    Game::Game(GameSpecs& specs)
        : m_specs(specs) {
            m_window = std::make_unique<Core::Window>(m_specs.windowSpec);
            m_window->create();
        }

    Game::~Game() {
        m_window->destroy();
    }

    void Game::run() {

        while (!m_window->shouldClose()) {
            m_window->pollEvents();
            m_logic->onUpdate();
            m_logic->onRender();
            m_window->update();
        }

    }

}
