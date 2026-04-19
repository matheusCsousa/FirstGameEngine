#pragma once

struct GLFWwindow;

namespace Core {
struct WindowSpecs {
    int width = 800;
    int height = 600;
};

class Window {
private:
    GLFWwindow* m_window = nullptr;
    WindowSpecs m_specs;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    Window(const WindowSpecs& specs);
    ~Window();

    GLFWwindow* getWindow() { return m_window; }

    void create();
    void destroy();
    void update();

    void swapBuffers();
    bool shouldClose();
    void setShouldClose(bool value);
    void pollEvents();

    void requestClose();
};
}
