#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Game.hpp>
#include <Camera.hpp>
#include <Input.hpp>
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
#include <Shader.hpp>
#include <Mesh.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
static_assert(std::has_virtual_destructor_v<Core::Graphics::Camera>);
static_assert(!std::is_copy_constructible_v<Core::Window>);
static_assert(!std::is_move_constructible_v<Core::Window>);
struct ProbeState { int updates = 0; int renders = 0; bool contextAtDestruction = false; };
class Probe final : public Core::Logic {
    ProbeState& state;
    GLFWwindow* native;
public:
    Probe(std::shared_ptr<Core::Window> window, ProbeState& state)
        : state(state), native(window->getWindow()) {
        require(glfwGetCurrentContext() == native, "Logic must be constructed with its context current");
        require(window->hasFramebuffer() && window->aspectRatio() > 0, "Framebuffer size was not initialized");
        Core::Render::VAO vao;
        vao.bind();
        const auto vaoId = vao.ID;
        const float vertices[33] = {};
        const unsigned int indices[] = {0, 1, 2};
        Core::Render::VBO vbo(vertices, sizeof(vertices));
        Core::Render::EBO ebo(indices, sizeof(indices));
        const auto vboId = vbo.ID;
        const auto eboId = ebo.ID;
        require(glIsBuffer(vboId) && glIsBuffer(eboId), "GPU buffers were not created");
        vbo.del(); vbo.del();
        ebo.del(); ebo.del();
        vao.del(); vao.del();
        require(vbo.ID == 0 && ebo.ID == 0 && vao.ID == 0, "Explicit deletion must clear ownership");
        require(!glIsBuffer(vboId) && !glIsBuffer(eboId) && !glIsVertexArray(vaoId), "GPU objects remained allocated");
        const auto assets = std::filesystem::path(FERSA_TEST_ASSETS);
        Core::Render::Shader shader((assets / "shader/shader.vert").string().c_str(),
                                    (assets / "shader/shader.frag").string().c_str());
        GLint linked = GL_FALSE;
        glGetProgramiv(shader.ID, GL_LINK_STATUS, &linked);
        require(linked == GL_TRUE, "Sample shaders must link");
        const auto program = shader.ID;
        shader.del(); shader.del();
        require(shader.ID == 0 && !glIsProgram(program), "Shader deletion must be idempotent");
        Core::Graphics::Mesh mesh(vertices, sizeof(vertices), indices, sizeof(indices));
        mesh.bind();
        GLint elementBuffer = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBuffer);
        require(elementBuffer != 0, "Mesh index buffer must belong to its VAO");
        mesh.unbind();
        require(glGetError() == GL_NO_ERROR, "Resource setup/deletion produced an OpenGL error");
    }
    ~Probe() override { state.contextAtDestruction = glfwGetCurrentContext() == native; }
    void onUpdate() override { ++state.updates; }
    void onRender() override { ++state.renders; glClear(GL_COLOR_BUFFER_BIT); }
};
void testWindows() {
    Core::WindowSpecs invalid;
    invalid.width = 0;
    bool rejected = false;
    try { Core::Window window(invalid); window.create(); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "Zero-width windows must be rejected");
    Core::WindowSpecs specs;
    specs.width = specs.height = 64;
    specs.visible = false;
    Core::Window first(specs), second(specs);
    first.create(); second.create();
    rejected = false;
    try { second.create(); } catch (const std::logic_error&) { rejected = true; }
    require(rejected, "Creating the same window twice must be rejected");
    first.destroy(); first.destroy();
    second.makeCurrent();
    require(glfwGetCurrentContext() == second.getWindow(), "Closing one window terminated another window's context");
    require(!second.shouldClose(), "Second window was invalidated");
}
}
int main() {
    try {
        testWindows();
        ProbeState state;
        {
            Core::GameSpecs specs;
            specs.windowSpec.visible = false;
            specs.windowSpec.width = specs.windowSpec.height = 64;
            Core::Game game(specs);
            bool rejected = false;
            try { game.run(1); } catch (const std::logic_error&) { rejected = true; }
            require(rejected, "Running without application logic must fail clearly");
            game.pushLogic<Probe>(state);
            game.run(2);
            require(state.updates == 2 && state.renders == 2, "Bounded game loop did not run exactly two frames");
        }
        require(state.contextAtDestruction, "Application resources were destroyed after the OpenGL context");
        require(glfwGetCurrentContext() == nullptr, "Window context remained current after shutdown");
        std::cout << "Runtime lifecycle checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
