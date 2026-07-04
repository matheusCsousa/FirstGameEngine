#pragma once

#include "../Event/Event.hpp"
#include <functional>

struct GLFWwindow;

namespace Core {
struct WindowSpecs {
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
    EventCallbackFn m_eventCallback;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    Window(const WindowSpecs& specs);
    ~Window();

    GLFWwindow* getWindow() { return m_window; }

    void setEventCallback(const EventCallbackFn& callback) { m_eventCallback = callback; }

    void create();
    void destroy();
    void update();

    void swapBuffers();
    bool shouldClose();
    void setShouldClose(bool value);
    void pollEvents();

    void requestClose();

    float aspectRatio();
};
}
