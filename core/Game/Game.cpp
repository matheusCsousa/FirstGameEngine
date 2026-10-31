#include "Game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "../Window/Window.hpp"
#include "../Input/Input.hpp"

namespace Core {

Game::Game(GameSpecs& specs)
    : m_specs(specs) {
        m_specs.windowSpec.title = m_specs.title;
        m_window = std::make_shared<Core::Window>(m_specs.windowSpec);
        m_window->create();
        m_window->setEventCallback(std::bind(&Game::onEvent, this, std::placeholders::_1));
        Core::Input::initialize(m_window->getWindow());
    }

Game::~Game() {
    // GPU owners must be released before their context is destroyed.
    m_window->makeCurrent();
    m_logic.reset();
    Core::Input::initialize(nullptr);
    m_window.reset();
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

void Game::run(std::size_t maxFrames) {
    if (!m_logic) throw std::logic_error("Register application logic before calling Game::run");
    m_window->makeCurrent();
    Core::Input::initialize(m_window->getWindow());
    std::size_t frames = 0;
    while (!m_window->shouldClose() && (maxFrames == 0 || frames < maxFrames)) {
        Core::Input::update();
        m_window->pollEvents();
        if (m_window->shouldClose()) break;
        if (!m_window->hasFramebuffer()) {
            glfwWaitEventsTimeout(0.05);
            continue;
        }
        m_logic->onUpdate();
        m_logic->onRender();
        m_window->update();
        ++frames;
    }
}
}
