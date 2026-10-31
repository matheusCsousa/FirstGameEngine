#include "Editor.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "../core/Graphics/Renderer/Texture/Texture.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
class EditorCamera final : public Core::Graphics::Camera {
public:
    EditorCamera() : Camera({6,4,8}, {0,1,0}, 55, 0.1f, 500) {
        yaw = -127; pitch = -22; updateCameraVectors();
    }
    void navigate(float dt, ImVec2 delta) {
        yaw += delta.x * 0.18f;
        pitch = std::clamp(pitch - delta.y * 0.18f, -89.0f, 89.0f);
        updateCameraVectors();
        auto& io = ImGui::GetIO();
        float speed = 5 * std::min(dt, 0.1f) * (io.KeyShift ? 3 : 1);
        if (ImGui::IsKeyDown(ImGuiKey_W)) position += front * speed;
        if (ImGui::IsKeyDown(ImGuiKey_S)) position -= front * speed;
        if (ImGui::IsKeyDown(ImGuiKey_D)) position += right * speed;
        if (ImGui::IsKeyDown(ImGuiKey_A)) position -= right * speed;
        if (ImGui::IsKeyDown(ImGuiKey_E)) position += worldUp * speed;
        if (ImGui::IsKeyDown(ImGuiKey_Q)) position -= worldUp * speed;
    }
};
void editVector(const char* label, glm::vec3& value, float speed, bool positive = false) {
    const auto previous = value;
    if (ImGui::DragFloat3(label, &value.x, speed, positive ? 0.01f : 0.0f,
                        positive ? 1000.0f : 0.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        for (int axis = 0; axis < 3; ++axis) {
            if (!std::isfinite(value[axis])) value[axis] = previous[axis];
            if (positive) value[axis] = std::clamp(value[axis], 0.01f, 1000.0f);
        }
    }
}
void defaultLayout(ImGuiID dock, ImVec2 size) {
    // DockBuilder is deliberately isolated: this internal API belongs to our pinned ImGui release.
    if (ImGui::DockBuilderGetNode(dock)) return;
    ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock, size);
    auto center = dock;
    auto left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.21f, nullptr, &center);
    auto right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.29f, nullptr, &center);
    ImGui::DockBuilderDockWindow("Hierarchy", left);
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Scene", center);
    ImGui::DockBuilderFinish(dock);
}
}

Editor::Editor(std::shared_ptr<Core::Window> target, const std::filesystem::path& assets) : window(std::move(target)) {
    scene = std::make_unique<Core::Scene>(std::make_unique<EditorCamera>());
    auto am = scene->getAssetManager();
    Core::Collision::AABB bounds;
    auto mesh = am->loadMeshFromOBJ("cube", (assets/"models/cube.obj").string(), bounds);
    auto shader = am->loadShader("surface", (assets/"shader/shader.vert").string().c_str(), (assets/"shader/shader.frag").string().c_str());
    auto texture = am->loadTexture("brick", (assets/"textures/brick.jpg").string().c_str(), "diffuse", 0);
    texture->texUnit(*shader, "tex0", 0);
    cubeMesh = mesh; cubeShader = shader; cubeTexture = texture; cubeBounds = bounds;
    auto cube = scene->createEntity(mesh, shader);
    cube->name = "Cube"; cube->position = {0,1,0}; cube->setLocalAABB(bounds); cube->addTexture(texture);
    selected = cube->id();
    auto ground = scene->duplicateEntity(selected);
    ground->name = "Ground"; ground->position = {0,-1,0}; ground->scale = {5,0.2f,5};
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Reproducible layout; layout persistence follows scene persistence.
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowPadding = {14,14}; style.FramePadding = {8,6}; style.ItemSpacing = {8,10};
    style.FrameRounding = 4; style.GrabRounding = 4;
    if (!ImGui_ImplGlfw_InitForOpenGL(window->getWindow(), true)) {
        ImGui::DestroyContext(); throw std::runtime_error("Editor GLFW backend failed");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
        throw std::runtime_error("Editor OpenGL backend failed");
    }
}
Editor::~Editor() {
    releaseNavigation();
    glDeleteFramebuffers(1, &framebuffer); glDeleteTextures(1, &color); glDeleteRenderbuffers(1, &depth);
    scene.reset();
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
}
void Editor::releaseNavigation() {
    if (navigating) glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    navigating = false;
}
void Editor::resizeViewport(int w, int h) {
    GLint limit; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
    w = std::clamp(w, 1, limit); h = std::clamp(h, 1, limit);
    if (w == width && h == height) return;
    if (!framebuffer) {
        glGenFramebuffers(1,&framebuffer); glGenTextures(1,&color); glGenRenderbuffers(1,&depth);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w,h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER,0); glBindRenderbuffer(GL_RENDERBUFFER,0); glBindTexture(GL_TEXTURE_2D,0);
    if (status != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Editor viewport framebuffer incomplete");
    width = w; height = h;
}
void Editor::createCube() {
    auto* cube = scene->createEntity(cubeMesh, cubeShader);
    cube->name = "Cube"; cube->position = {0,1,0};
    cube->setLocalAABB(cubeBounds); cube->addTexture(cubeTexture);
    selected = cube->id();
}
void Editor::onRender() {
    ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
    if (navigating && (!ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
        !glfwGetWindowAttrib(window->getWindow(), GLFW_FOCUSED) || ImGui::IsKeyPressed(ImGuiKey_Escape))) releaseNavigation();
    auto* viewport = ImGui::GetMainViewport();
    auto dock = ImGui::GetID("FersaDock");
    defaultLayout(dock, viewport->WorkSize);
    ImGui::DockSpaceOverViewport(dock, viewport);
    ImGui::Begin("Hierarchy");
    ImGui::TextDisabled("UNTITLED SCENE");
    if (ImGui::Button("+ Cube")) createCube();
    ImGui::Separator();
    for (auto& entity : scene->getEntities()) {
        ImGui::PushID(static_cast<int>(entity->id()));
        if (ImGui::Selectable((entity->name.empty() ? "(unnamed)" : entity->name.c_str()), selected == entity->id())) selected = entity->id();
        ImGui::PopID();
    }
    ImGui::Spacing(); ImGui::TextDisabled("%zu objects", scene->getEntities().size());
    ImGui::TextWrapped("Editing mode: physics paused.\nSession only: changes are not saved.");
    ImGui::End();
    ImGui::Begin("Inspector");
    if (auto* entity = scene->findEntity(selected)) {
        ImGui::TextDisabled("OBJECT");
        ImGui::InputText("Name", &entity->name);
        ImGui::SeparatorText("Transform");
        editVector("Position", entity->position, 0.05f);
        editVector("Rotation", entity->rotation, 0.5f);
        ImGui::TextDisabled("Rotation in degrees");
        editVector("Scale", entity->scale, 0.01f, true);
        if (ImGui::Button("Reset transform")) { entity->position={}; entity->rotation={}; entity->scale={1,1,1}; }
        ImGui::Separator();
        if (ImGui::Button("Duplicate")) selected = scene->duplicateEntity(selected)->id();
        // Read selection again next frame: no raw pointer survives deletion.
        if (ImGui::Button("Delete")) { scene->removeEntity(selected); selected = 0; }
    } else { selected = 0; ImGui::TextWrapped("Select an object in the hierarchy to edit its transform."); }
    ImGui::End();
    bool sceneVisible = ImGui::Begin("Scene");
    ImGui::TextDisabled("RMB + WASD: fly   Q/E: down/up   Shift: faster");
    auto available = ImGui::GetContentRegionAvail();
    if (sceneVisible && available.x >= 1 && available.y >= 1) {
        auto& io = ImGui::GetIO();
        resizeViewport(static_cast<int>(available.x * io.DisplayFramebufferScale.x), static_cast<int>(available.y * io.DisplayFramebufferScale.y));
        ImGui::Image(static_cast<ImTextureID>(color), available, {0,1}, {1,0});
        bool started = !navigating && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantTextInput;
        if (started) { navigating = true; glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
        if (navigating) static_cast<EditorCamera*>(scene->getCamera())->navigate(io.DeltaTime, started ? ImVec2{} : io.MouseDelta);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); glViewport(0,0,width,height);
        glEnable(GL_DEPTH_TEST); glClearColor(0.10f,0.12f,0.16f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        scene->render(static_cast<float>(width)/height);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    } else releaseNavigation();
    ImGui::End();
    ImGui::Render();
    int w,h; glfwGetFramebufferSize(window->getWindow(), &w,&h); glViewport(0,0,w,h);
    glClearColor(0.08f,0.09f,0.11f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
