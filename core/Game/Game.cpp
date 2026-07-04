#include "Game.hpp"

#include <GL/glew.h>

#include "../Window/Window.hpp"
#include "../Input/Input.hpp"

namespace Core {

Game::Game(GameSpecs& specs)
    : m_specs(specs) {
        m_window = std::make_unique<Core::Window>(m_specs.windowSpec);
        m_window->create();
        m_window->setEventCallback(std::bind(&Game::onEvent, this, std::placeholders::_1));
        Core::Input::initialize(m_window->getWindow());
    }

Game::~Game() {
    m_window->destroy();
}

void Game::onEvent(Event& event) {
    Core::Input::onEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Game::onWindowClose, this, std::placeholders::_1));

    if (m_logic && !event.Handled) {
        m_logic->onEvent(event);
    }
}

bool Game::onWindowClose(WindowCloseEvent& event) {
    (void)event;
    m_window->requestClose();
    return true;
}

void Game::run() {

    while (!m_window->shouldClose()) {
        Core::Input::update();
        m_window->pollEvents();
        m_logic->onUpdate();
        m_logic->onRender();
        m_window->update();
    }
}
}
