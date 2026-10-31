#include "Editor.hpp"
#include "TransformControls.hpp"
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
#include <utility>

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
    dirty = true;
}
void Editor::onRender() {
    ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
    if (navigating && (!ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
        !glfwGetWindowAttrib(window->getWindow(), GLFW_FOCUSED) || ImGui::IsKeyPressed(ImGuiKey_Escape))) releaseNavigation();
    fileControls();
    auto* viewport = ImGui::GetMainViewport();
    auto dock = ImGui::GetID("FersaDock");
    defaultLayout(dock, viewport->WorkSize);
    ImGui::DockSpaceOverViewport(dock, viewport);
    ImGui::Begin("Hierarchy");
    ImGui::TextWrapped("%s%s", currentPath.empty() ? "Untitled scene" : currentPath.filename().string().c_str(), dirty ? " *" : "");
    if (ImGui::Button("+ Cube")) createCube();
    ImGui::Separator();
    for (auto& entity : scene->getEntities()) {
        ImGui::PushID(std::to_string(entity->id()).c_str());
        if (ImGui::Selectable((entity->name.empty() ? "(unnamed)" : entity->name.c_str()), selected == entity->id())) selected = entity->id();
        ImGui::PopID();
    }
    ImGui::Spacing(); ImGui::TextDisabled("%zu objects", scene->getEntities().size());
    ImGui::TextWrapped("Editing mode: physics paused.");
    ImGui::TextWrapped("%s", dirty ? "Unsaved changes" : "No unsaved changes");
    if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
    ImGui::End();
    ImGui::Begin("Inspector");
    if (auto* entity = scene->findEntity(selected)) {
        ImGui::TextDisabled("OBJECT");
        ImGui::PushID(std::to_string(selected).c_str());
        dirty |= ImGui::InputText("Name", &entity->name);
        ImGui::SeparatorText("Transform");
        dirty |= EditorWidgets::editVector("Position", entity->position, 0.05f);
        dirty |= EditorWidgets::editVector("Rotation", entity->rotation, 0.5f);
        ImGui::TextDisabled("Rotation in degrees");
        dirty |= EditorWidgets::editVector("Scale", entity->scale, 0.01f, true);
        ImGui::TextWrapped("Drag values horizontally, or Ctrl+click to type. Axes: X / Y / Z.");
        if (ImGui::Button("Reset transform")) { entity->position={}; entity->rotation={}; entity->scale={1,1,1}; dirty=true; }
        ImGui::Separator();
        if (ImGui::Button("Duplicate")) { selected = scene->duplicateEntity(selected)->id(); dirty=true; }
        // Read selection again next frame: no raw pointer survives deletion.
        if (ImGui::Button("Delete")) { scene->removeEntity(selected); selected = 0; dirty=true; }
        ImGui::PopID();
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

namespace {
std::string pathText(const std::filesystem::path& path) {
    auto text=path.u8string(); return {text.begin(),text.end()};
}
}
void Editor::request(Action action) {
    if (pending != Action::None || fileMode != FileMode::None) return;
    releaseNavigation();
    pending=action;
    if (dirty) showUnsaved=true;
    else executePending();
}
bool Editor::onCloseRequested() {
    if (!dirty && pending==Action::None && fileMode==FileMode::None) return true;
    request(Action::Close);
    return false;
}
void Editor::executePending() {
    const auto action=std::exchange(pending,Action::None);
    if (action==Action::New) {
        scene=std::make_unique<Core::Scene>(std::make_unique<EditorCamera>());
        selected=0; dirty=false; currentPath.clear(); status="New empty scene";
    } else if (action==Action::Open) openFileDialog(FileMode::Open);
    else if (action==Action::Close) window->requestClose();
}
void Editor::openFileDialog(FileMode mode) {
    releaseNavigation();
    fileMode=mode; showFile=true; fileError.clear(); replaceExisting=false;
    pathInput=pathText(currentPath.empty() ? std::filesystem::current_path()/"scene.fersa.json" : currentPath);
}
bool Editor::saveTo(const std::filesystem::path& path) {
    try {
        const auto resolved=std::filesystem::absolute(path).lexically_normal();
        EditorFiles::write(resolved,EditorFiles::capture(*scene));
        currentPath=resolved; dirty=false; status="Saved "+pathText(resolved); fileError.clear(); return true;
    } catch (const std::exception& error) { fileError=error.what(); status="Save failed: "+fileError; return false; }
}
bool Editor::loadFrom(const std::filesystem::path& path) {
    try {
        const auto resolved=std::filesystem::absolute(path).lexically_normal();
        const auto document=EditorFiles::read(resolved);
        auto replacement=std::make_unique<Core::Scene>(std::make_unique<EditorCamera>());
        for (const auto& object : document.objects) {
            auto* entity=replacement->createEntity(cubeMesh,cubeShader,object.id);
            entity->name=object.name; entity->position=object.position; entity->rotation=object.rotation; entity->scale=object.scale;
            entity->velocity=object.velocity; entity->acceleration=object.acceleration;
            entity->mass=object.mass; entity->useGravity=object.gravity;
            entity->setLocalAABB(cubeBounds); entity->addTexture(cubeTexture);
        }
        // Commit only after the entire document has loaded. Invalid files preserve the current scene.
        scene=std::move(replacement); selected=0; currentPath=resolved; dirty=false;
        status="Opened "+pathText(resolved); fileError.clear(); return true;
    } catch (const std::exception& error) { fileError=error.what(); status="Open failed: "+fileError; return false; }
}
void Editor::fileControls() {
    const bool busy=pending!=Action::None || fileMode!=FileMode::None || showUnsaved;
    auto save=[&] {
        if (currentPath.empty()) openFileDialog(FileMode::Save);
        else saveTo(currentPath);
    };
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::BeginDisabled(busy);
            if (ImGui::MenuItem("New scene","Ctrl+N")) request(Action::New);
            if (ImGui::MenuItem("Open...","Ctrl+O")) request(Action::Open);
            if (ImGui::MenuItem("Save","Ctrl+S")) save();
            if (ImGui::MenuItem("Save as...","Ctrl+Shift+S")) openFileDialog(FileMode::Save);
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) request(Action::Close);
            ImGui::EndDisabled(); ImGui::EndMenu();
        }
        ImGui::TextDisabled("  FERSA  /  EDIT MODE%s", dirty ? "  /  UNSAVED" : "");
        ImGui::EndMainMenuBar();
    }
    auto& io=ImGui::GetIO();
    if (!busy && !navigating && io.KeyCtrl && !io.WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_S,false)) { if (io.KeyShift) openFileDialog(FileMode::Save); else save(); }
        else if (ImGui::IsKeyPressed(ImGuiKey_O,false)) request(Action::Open);
        else if (ImGui::IsKeyPressed(ImGuiKey_N,false)) request(Action::New);
    }
    if (showUnsaved) { ImGui::OpenPopup("Unsaved changes"); showUnsaved=false; fileError.clear(); }
    ImGui::SetNextWindowSize({460,0},ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Unsaved changes",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Save this scene before continuing? Unsaved changes will be lost if you discard them.");
        if (!fileError.empty()) ImGui::TextWrapped("%s",fileError.c_str());
        if (ImGui::Button("Save")) {
            if (currentPath.empty()) { openFileDialog(FileMode::Save); ImGui::CloseCurrentPopup(); }
            else if (saveTo(currentPath)) { ImGui::CloseCurrentPopup(); executePending(); }
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")) { ImGui::CloseCurrentPopup(); executePending(); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            pending=Action::None; ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (showFile) { ImGui::OpenPopup("Scene file"); showFile=false; }
    ImGui::SetNextWindowSize({650,0},ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Scene file",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(fileMode==FileMode::Save ? "Save scene" : "Open scene");
        ImGui::TextWrapped("Enter a full path to a .fersa.json file. The destination folder must already exist.");
        ImGui::SetNextItemWidth(-1); ImGui::InputText("##Path",&pathInput);
        if (fileMode==FileMode::Save) ImGui::Checkbox("Replace existing file",&replaceExisting);
        if (!fileError.empty()) ImGui::TextWrapped("%s",fileError.c_str());
        if (ImGui::Button(fileMode==FileMode::Save ? "Save file" : "Open file")) {
            try {
                if (pathInput.empty()) throw std::runtime_error("Enter a file path");
                const auto path=std::filesystem::absolute(std::filesystem::path(std::u8string(pathInput.begin(),pathInput.end()))).lexically_normal();
                if (fileMode==FileMode::Save && path!=currentPath && std::filesystem::exists(path) && !replaceExisting)
                    throw std::runtime_error("File exists. Enable Replace existing file to overwrite it.");
                const bool success=fileMode==FileMode::Save ? saveTo(path) : loadFrom(path);
                if (success) { fileMode=FileMode::None; ImGui::CloseCurrentPopup(); executePending(); }
            } catch (const std::exception& error) { fileError=error.what(); }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            fileMode=FileMode::None; pending=Action::None; ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    const auto title="Fersa - "+(currentPath.empty() ? std::string("Untitled scene") : pathText(currentPath.filename()))+(dirty ? " *" : "");
    glfwSetWindowTitle(window->getWindow(),title.c_str());
}
