#include "../core/Scene/Scene.hpp"
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
#include <Texture.hpp>
#include <array>
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
class CloseProbe final : public Core::Logic {
    std::shared_ptr<Core::Window> window;
    int& attempts;
public:
    CloseProbe(std::shared_ptr<Core::Window> window,int& attempts) : window(window),attempts(attempts) {}
    bool onCloseRequested() override { return ++attempts>1; }
    void onUpdate() override {
        // Reproduce GLFW's native close semantics: it sets should-close before invoking the callback.
        auto* native=window->getWindow();
        auto callback=glfwSetWindowCloseCallback(native,nullptr);
        glfwSetWindowCloseCallback(native,callback);
        glfwSetWindowShouldClose(native,GLFW_TRUE);
        callback(native);
        require(window->shouldClose()==(attempts>1),"Application close veto was not respected");
    }
    void onRender() override {}
};
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
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT,8);
            const auto fixtures=std::filesystem::path(FERSA_TEST_FIXTURES);
            Core::Render::Texture rgb((fixtures/"rgb-odd.png").string().c_str(),"diffuse",0);
            GLint alignment;glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);require(alignment==8,"Texture upload must restore pixel alignment");
            rgb.bind();std::array<unsigned char,12> pixels{};glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            require(pixels[0]==255&&pixels[1]==0&&pixels[3]==255&&pixels[5]==255&&pixels[10]==255,"Odd-width RGB texture upload is incorrect");
            Core::Render::Texture gray((fixtures/"gray-alpha.png").string().c_str(),"diffuse",0);gray.bind();
            std::array<unsigned char,4> ga{};glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,ga.data());
            require(ga[0]==96&&ga[1]==96&&ga[2]==96&&ga[3]==128,"Two-channel texture must decode as grayscale plus alpha");
            glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        }
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
        {
            Core::Scene scene(std::make_unique<Core::Graphics::Camera>());
            auto* original = scene.createEntity(nullptr, nullptr);
            const auto originalId = original->id();
            original->name = "Example"; original->position = {1,2,3}; original->useGravity = true;
            auto* copy = scene.duplicateEntity(originalId);
            const auto copyId = copy->id();
            require(originalId != 0 && copyId != originalId, "Scene IDs must be unique and nonzero");
            require(copy->name == "Example Copy" && copy->position == original->position && copy->useGravity,
                    "Duplicate must preserve editable properties");
            copy->position.x = 10;
            require(original->position.x == 1, "Duplicate transforms must be independent");
            require(scene.removeEntity(originalId) && !scene.findEntity(originalId), "Deleted selection must resolve to null");
            require(!scene.removeEntity(originalId) && !scene.duplicateEntity(originalId), "Missing IDs must be handled safely");
            require(scene.findEntity(copyId) == copy, "Removing another entity must preserve identity");
            const auto replacementId = scene.createEntity(nullptr, nullptr)->id();
            require(replacementId > copyId, "Deleted IDs must not be reused");
            auto* restored=scene.createEntity(nullptr,nullptr,1000);
            require(restored->id()==1000 && scene.createEntity(nullptr,nullptr)->id()>1000,"Restored IDs must advance the allocator");
            bool duplicateRejected=false;
            try { scene.createEntity(nullptr,nullptr,1000); } catch(const std::invalid_argument&) { duplicateRejected=true; }
            require(duplicateRejected,"Restored duplicate IDs must be rejected");
        }
        {
            int attempts=0;
            Core::GameSpecs specs; specs.windowSpec.visible=false;
            Core::Game game(specs); game.pushLogic<CloseProbe>(attempts); game.run(4);
            require(attempts==2,"Close cancellation must allow later frames and a second close request");
        }
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
