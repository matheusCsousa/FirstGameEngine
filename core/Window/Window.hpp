#pragma once

#include "../Event/Event.hpp"
#include <functional>
#include <string>

struct GLFWwindow;

namespace Core {
struct WindowSpecs {
    std::string title = "Fersa Engine";
    bool visible = true;
    int width = 800;
    int height = 600;
    int bufferWidth = 800;
    int bufferHeight = 600;
};

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

private:
    GLFWwindow* m_window = nullptr;
    WindowSpecs m_specs;
    bool m_ownsGlfw = false;
    EventCallbackFn m_eventCallback;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    Window(const WindowSpecs& specs);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    GLFWwindow* getWindow() { return m_window; }

    void setEventCallback(const EventCallbackFn& callback) { m_eventCallback = callback; }

    void create();
    void destroy() noexcept;
    void makeCurrent() noexcept;
    bool hasFramebuffer() const;
    void update();

    void swapBuffers();
    bool shouldClose();
    void setShouldClose(bool value);
    void pollEvents();

    void requestClose();

    float aspectRatio() const;
};
}
