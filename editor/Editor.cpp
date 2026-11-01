#include "Editor.hpp"
#include "TransformControls.hpp"
#include "EditorMath.hpp"
#include "NativeFiles.hpp"
#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>
#include "../core/Input/Input.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuizmo.h>
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
std::unordered_map<GLFWwindow*,Editor*> dropEditors;
class EditorCamera final : public Core::Graphics::Camera {
public:
    EditorCamera() : Camera({6,4,8}, {0,1,0}, 55, 0.1f, 500) {
        yaw = -127; pitch = -22; updateCameraVectors();
    }
    void focus(const Core::Collision::AABB& bounds) {
        const auto center=(bounds.min+bounds.max)*0.5f;
        const float radius=std::max(0.1f,glm::length(bounds.max-bounds.min)*0.5f);
        position=center-front*(radius*2.8f);nearPlane=std::max(0.01f,radius/1000);farPlane=std::max(500.0f,radius*20);
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
    auto bottom=ImGui::DockBuilderSplitNode(center,ImGuiDir_Down,0.30f,nullptr,&center);
    ImGui::DockBuilderDockWindow("Assets",bottom);
    ImGui::DockBuilderDockWindow("Hierarchy", left);
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Scene", center);
    ImGui::DockBuilderFinish(dock);
}
}

Editor::Editor(std::shared_ptr<Core::Window> target, const std::filesystem::path& assetRoot,const std::filesystem::path& project) : window(std::move(target)) {
    scene = std::make_unique<Core::Scene>(std::make_unique<EditorCamera>());
    const auto projectRoot=project.empty()?std::filesystem::absolute(assetRoot).parent_path()/"project":std::filesystem::absolute(project);
    std::filesystem::create_directories(projectRoot);
    assets=std::make_unique<AssetLibrary>(assetRoot,projectRoot);
    const auto loaded=assets->mesh("models/cube.obj");
    auto mesh=loaded.mesh;const auto bounds=loaded.bounds;
    auto shader=assets->shader();auto texture=assets->texture("builtin/brick");
    cubeMesh=mesh;cubeShader=shader;cubeTexture=texture;cubeBounds=bounds;
    auto cube = scene->createEntity(mesh, shader);
    cube->name = "Cube"; cube->position = {0,1,0}; cube->setLocalAABB(bounds); cube->addTexture(texture);
    cube->mass=1; cube->useGravity=true;
    selected = cube->id();
    auto ground = scene->duplicateEntity(selected);
    ground->name = "Ground"; ground->position = {0,-1,0}; ground->scale = {5,0.2f,5};
    ground->mass=0; ground->useGravity=false;
    history.reset(EditorFiles::capture(*scene),selected);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
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
    dropEditors[window->getWindow()]=this;
    previousDrop=glfwSetDropCallback(window->getWindow(),[](GLFWwindow* native,int count,const char** paths){
        auto found=dropEditors.find(native);if(found==dropEditors.end())return;
        for(int i=0;i<count;++i)found->second->pendingImports.push_back(AssetPaths::path(paths[i]));
    });
    refreshAssets();
}
Editor::~Editor() {
    glfwSetDropCallback(window->getWindow(),previousDrop);dropEditors.erase(window->getWindow());
    releaseNavigation();
    glDeleteFramebuffers(1, &framebuffer); glDeleteTextures(1, &color); glDeleteRenderbuffers(1, &depth);
    play.stop();
    scene.reset();
    assets.reset();
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
    flushHistory();
    auto* cube = scene->createEntity(cubeMesh, cubeShader);
    cube->name = "Cube"; cube->position = {0,1,0};
    cube->setLocalAABB(cubeBounds); cube->addTexture(cubeTexture);
    selected = cube->id();
    dirty = true;
}
void Editor::startPlay() {
    if (play.isPlaying()) return;
    flushHistory();
    releaseNavigation();
    try {
        play.start(*scene,std::make_unique<EditorCamera>(*static_cast<EditorCamera*>(scene->getCamera())),assets->root());
        status="Play: scripts and physics run on a scene copy. Stop restores authored data.";
    } catch (const std::exception& error) { status="Cannot start preview: "+std::string(error.what()); }
}
void Editor::stopPlay() {
    if (!play.isPlaying()) return;
    releaseNavigation(); play.stop();
    status="Stopped: authored scene restored.";
}
void Editor::onUpdate() {
    if (play.isPlaying()) play.advance(Core::Input::getDeltaTime());
}
void Editor::onRender() {
    ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    editFinished=false;
    if (navigating && (!ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
        !glfwGetWindowAttrib(window->getWindow(), GLFW_FOCUSED) || ImGui::IsKeyPressed(ImGuiKey_Escape))) releaseNavigation();
    if(!pendingImports.empty()){auto files=std::move(pendingImports);pendingImports.clear();importAssets(files);}
    fileControls();
    auto* viewport = ImGui::GetMainViewport();
    auto dock = ImGui::GetID("FersaDock");
    defaultLayout(dock, viewport->WorkSize);
    ImGui::DockSpaceOverViewport(dock, viewport);
    ImGui::Begin("Hierarchy");
    ImGui::TextWrapped("%s%s", currentPath.empty() ? "Untitled scene" : currentPath.filename().string().c_str(), dirty ? " *" : "");
    ImGui::BeginDisabled(play.isPlaying());
    if (ImGui::Button("+ Cube")) createCube();
    ImGui::EndDisabled();
    ImGui::Separator();
    for (auto& entity : displayedScene().getEntities()) {
        ImGui::PushID(std::to_string(entity->id()).c_str());
        if (ImGui::Selectable((entity->name.empty() ? "(unnamed)" : entity->name.c_str()), selected == entity->id())) selected = entity->id();
        ImGui::PopID();
    }
    ImGui::Spacing(); ImGui::TextDisabled("%zu objects", displayedScene().getEntities().size());
    ImGui::TextWrapped("%s", play.isPlaying() ? (play.isPaused() ? "Preview paused" : "Physics preview running") : "Editing mode: physics paused.");
    ImGui::TextWrapped("%s", dirty ? "Unsaved changes" : "No unsaved changes");
    if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
    ImGui::End();
    ImGui::Begin("Inspector");
    if (auto* entity = displayedScene().findEntity(selected)) {
        if (play.isPlaying()) ImGui::TextWrapped("Runtime values (read only). Stop to edit.");
        ImGui::BeginDisabled(play.isPlaying());
        ImGui::TextDisabled("OBJECT");
        ImGui::PushID(std::to_string(selected).c_str());
        dirty |= ImGui::InputText("Name", &entity->name);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::SeparatorText("Transform");
        dirty |= EditorWidgets::editVector("Position", entity->position, 0.05f);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        dirty |= EditorWidgets::editVector("Rotation", entity->rotation, 0.5f);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::TextDisabled("Rotation in degrees");
        dirty |= EditorWidgets::editVector("Scale", entity->scale, 0.01f, true);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::TextWrapped("Drag values horizontally, or Ctrl+click to type. Axes: X / Y / Z.");
        if (ImGui::Button("Reset transform")) { flushHistory(); entity->position={}; entity->rotation={}; entity->scale={1,1,1}; dirty=true; }
        drawMaterialAndScript(*entity);
        ImGui::SeparatorText("Physics");
        const float previousMass=entity->mass;
        if (ImGui::DragFloat("Mass (kg)",&entity->mass,0.1f,0,1000000,"%.2f",ImGuiSliderFlags_AlwaysClamp)) {
            if (!std::isfinite(entity->mass)) entity->mass=previousMass;
            dirty |= entity->mass!=previousMass;
        }
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        dirty |= ImGui::Checkbox("Use gravity",&entity->useGravity);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::TextWrapped("Mass 0 is static. Set mass above 0 for movement.");
        ImGui::TextUnformatted(play.isPlaying() ? "Velocity (X / Y / Z)" : "Initial velocity (X / Y / Z)");
        ImGui::SetNextItemWidth(-1);
        dirty |= EditorWidgets::editVector("##Velocity",entity->velocity,0.1f);
        editFinished |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::Separator();
        if (ImGui::Button("Duplicate")) { flushHistory(); selected = scene->duplicateEntity(selected)->id(); dirty=true; }
        // Read selection again next frame: no raw pointer survives deletion.
        if (ImGui::Button("Delete")) { flushHistory(); scene->removeEntity(selected); selected = 0; dirty=true; }
        ImGui::PopID();
        ImGui::EndDisabled();
    } else { selected = 0; ImGui::TextWrapped("Select an object in the hierarchy to edit its transform."); }
    ImGui::End();
    drawAssets();
    bool sceneVisible = ImGui::Begin("Scene");
    ImGui::BeginDisabled(play.isPlaying());
    if(ImGui::RadioButton("Move",transformTool==TransformTool::Move)) transformTool=TransformTool::Move;
    ImGui::SameLine(); if(ImGui::RadioButton("Rotate",transformTool==TransformTool::Rotate)) transformTool=TransformTool::Rotate;
    ImGui::SameLine(); if(ImGui::RadioButton("Scale",transformTool==TransformTool::Scale)) transformTool=TransformTool::Scale;
    ImGui::SameLine();
    bool shownLocal=transformTool==TransformTool::Scale || localSpace;
    ImGui::BeginDisabled(transformTool==TransformTool::Scale);
    if(ImGui::Checkbox("Local axes",&shownLocal))localSpace=shownLocal;
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    ImGui::TextDisabled("W/E/R: tools | F: frame | RMB + WASD: fly");
    auto available = ImGui::GetContentRegionAvail();
    if (sceneVisible && available.x >= 1 && available.y >= 1) {
        auto& io = ImGui::GetIO();
        resizeViewport(static_cast<int>(available.x * io.DisplayFramebufferScale.x), static_cast<int>(available.y * io.DisplayFramebufferScale.y));
        auto imageOrigin=ImGui::GetCursorScreenPos();
        ImGui::Image(static_cast<ImTextureID>(color), available, {0,1}, {1,0});
        const bool imageHovered=ImGui::IsItemHovered();
        bool started = !navigating && imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantTextInput;
        if (started) { navigating = true; glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
        if (navigating) static_cast<EditorCamera*>(displayedScene().getCamera())->navigate(io.DeltaTime, started ? ImVec2{} : io.MouseDelta);
        if(!play.isPlaying()&&ImGui::BeginDragDropTarget()) {
            if(auto* payload=ImGui::AcceptDragDropPayload("FERSA_ASSET")) {
                const auto reference=std::string(static_cast<const char*>(payload->Data));
                try {
                    if(AssetLibrary::kind(reference)==AssetKind::Model)createModel(reference);
                    else {
                        auto* camera=scene->getCamera();auto ray=EditorMath::viewportRay({io.MousePos.x,io.MousePos.y},{imageOrigin.x,imageOrigin.y},{available.x,available.y},camera->calcViewMatrix(),camera->calcProjectionMatrix(static_cast<float>(width)/height));
                        auto target=ray?EditorMath::pick(*scene,*ray):0;assignAsset(reference,target);
                    }
                }catch(const std::exception& error){assetMessage=error.what();}
            }
            ImGui::EndDragDropTarget();
        }
        viewportTools({imageOrigin.x,imageOrigin.y},{available.x,available.y},imageHovered);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); glViewport(0,0,width,height);
        glEnable(GL_DEPTH_TEST); glClearColor(0.10f,0.12f,0.16f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        displayedScene().render(static_cast<float>(width)/height);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    } else { releaseNavigation(); ImGuizmo::Enable(false); gizmoUsing=false; }
    ImGui::End();
    if(!play.isPlaying()) {
        auto document=EditorFiles::capture(*scene);
        dirty=history.isDirty(document);
        if(!gizmoUsing && (editFinished || !ImGui::IsAnyItemActive())) history.commit(std::move(document),selected);
    }
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
    stopPlay();
    flushHistory();
    releaseNavigation();
    pending=action;
    if (dirty) showUnsaved=true;
    else executePending();
}
bool Editor::onCloseRequested() {
    if(!editingScript.empty()){scriptError="Close the script editor before exiting.";return false;}
    stopPlay();
    flushHistory();
    if (!dirty && pending==Action::None && fileMode==FileMode::None) return true;
    request(Action::Close);
    return false;
}
void Editor::executePending() {
    const auto action=std::exchange(pending,Action::None);
    if (action==Action::New) {
        scene=std::make_unique<Core::Scene>(std::make_unique<EditorCamera>());
        selected=0; dirty=false; currentPath.clear(); status="New empty scene";
        history.reset(EditorFiles::capture(*scene),selected);
    } else if (action==Action::Open) openFileDialog(FileMode::Open);
    else if (action==Action::Close) window->requestClose();
}
void Editor::openFileDialog(FileMode mode) {
    releaseNavigation();
    fileMode=mode; showFile=true; fileError.clear(); replaceExisting=false;
    pathInput=pathText(currentPath.empty() ? assets->root()/"scene.fersa.json" : currentPath);
}
bool Editor::saveTo(const std::filesystem::path& path) {
    if (play.isPlaying()) { status="Stop the preview before saving."; return false; }
    flushHistory();
    try {
        const auto resolved=std::filesystem::absolute(path).lexically_normal();
        auto document=EditorFiles::capture(*scene);
        assets->copyReferenced(document,resolved.parent_path());
        auto newLibrary=std::make_unique<AssetLibrary>(assets->builtinRoot(),resolved.parent_path());
        EditorFiles::write(resolved,document);
        assets=std::move(newLibrary);refreshAssets();
        history.markSaved(EditorFiles::capture(*scene));
        currentPath=resolved; dirty=false; status="Saved "+pathText(resolved); fileError.clear(); return true;
    } catch (const std::exception& error) { fileError=error.what(); status="Save failed: "+fileError; return false; }
}
bool Editor::loadFrom(const std::filesystem::path& path) {
    try {
        const auto resolved=std::filesystem::absolute(path).lexically_normal();
        const auto document=EditorFiles::read(resolved);
        auto library=std::make_unique<AssetLibrary>(assets->builtinRoot(),resolved.parent_path());
        auto replacement=makeScene(document,std::make_unique<EditorCamera>(),library.get());
        // Commit only after the entire document has loaded. Invalid files preserve the current scene.
        scene=std::move(replacement); assets=std::move(library);refreshAssets(); selected=0; currentPath=resolved; dirty=false;
        history.reset(EditorFiles::capture(*scene),selected);
        status="Opened "+pathText(resolved); fileError.clear(); return true;
    } catch (const std::exception& error) { fileError=error.what(); status="Open failed: "+fileError; return false; }
}
void Editor::fileControls() {
    const bool busy=pending!=Action::None || fileMode!=FileMode::None || showUnsaved || !editingScript.empty() || showImport || ImGui::IsPopupOpen("Import file");
    auto save=[&] {
        if (currentPath.empty()) openFileDialog(FileMode::Save);
        else saveTo(currentPath);
    };
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::BeginDisabled(busy || play.isPlaying());
            if (ImGui::MenuItem("New scene","Ctrl+N")) request(Action::New);
            if (ImGui::MenuItem("Open...","Ctrl+O")) request(Action::Open);
            if (ImGui::MenuItem("Save","Ctrl+S")) save();
            if (ImGui::MenuItem("Save as...","Ctrl+Shift+S")) openFileDialog(FileMode::Save);
            ImGui::Separator();
            ImGui::EndDisabled();
            if (ImGui::MenuItem("Exit",nullptr,false,!busy)) request(Action::Close);
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit")) {
            if(!play.isPlaying()) flushHistory();
            if(ImGui::MenuItem("Undo","Ctrl+Z",false,!busy && !play.isPlaying() && history.canUndo())) restoreHistory(false);
            if(ImGui::MenuItem("Redo","Ctrl+Y / Ctrl+Shift+Z",false,!busy && !play.isPlaying() && history.canRedo())) restoreHistory(true);
            ImGui::EndMenu();
        }
        ImGui::BeginDisabled(busy);
        if (!play.isPlaying()) {
            if (ImGui::Button("Play")) startPlay();
        } else {
            if (ImGui::Button("Stop")) stopPlay();
            if (play.isPlaying()) {
                if (ImGui::Button(play.isPaused() ? "Resume" : "Pause")) play.setPaused(!play.isPaused());
                ImGui::BeginDisabled(!play.isPaused());
                if (ImGui::Button("Step")) play.step();
                ImGui::EndDisabled();
            }
        }
        ImGui::EndDisabled();
        ImGui::TextDisabled("  FERSA  /  %s%s", play.isPlaying() ? (play.isPaused() ? "PAUSED" : "PHYSICS PREVIEW") : "EDIT MODE", dirty ? "  /  UNSAVED" : "");
        ImGui::EndMainMenuBar();
    }
    auto& io=ImGui::GetIO();
    if (!busy && !play.isPlaying() && !navigating && io.KeyCtrl) {
        if(!io.WantTextInput && !ImGui::IsAnyItemActive() && !gizmoUsing && ImGui::IsKeyPressed(ImGuiKey_Z,false)) restoreHistory(io.KeyShift);
        else if(!io.WantTextInput && !ImGui::IsAnyItemActive() && !gizmoUsing && ImGui::IsKeyPressed(ImGuiKey_Y,false)) restoreHistory(true);
        else if (ImGui::IsKeyPressed(ImGuiKey_S,false)) { if (io.KeyShift) openFileDialog(FileMode::Save); else save(); }
        else if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_O,false)) request(Action::Open);
        else if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_N,false)) request(Action::New);
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
    const auto title="Fersa - "+(currentPath.empty() ? std::string("Untitled scene") : pathText(currentPath.filename()))+(dirty ? " *" : "")+(play.isPlaying() ? " [Preview]" : "");
    glfwSetWindowTitle(window->getWindow(),title.c_str());
}

std::unique_ptr<Core::Scene> Editor::makeScene(const EditorFiles::Document& document,std::unique_ptr<Core::Graphics::Camera> camera,AssetLibrary* library) {
    if(!library)library=assets.get();
    auto result=std::make_unique<Core::Scene>(std::move(camera));
    for(const auto& object:document.objects) {
        const auto mesh=library->mesh(object.mesh);auto texture=library->texture(object.material);
        if(!object.script.empty())library->scriptSource(object.script);
        auto* entity=result->createEntity(mesh.mesh,library->shader(),object.id);
        entity->meshAsset=object.mesh;entity->materialAsset=object.material;entity->scriptAsset=object.script;entity->tint=object.tint;entity->tiling=object.tiling;
        entity->name=object.name;entity->position=object.position;entity->rotation=object.rotation;entity->scale=object.scale;
        entity->velocity=object.velocity;entity->acceleration=object.acceleration;
        entity->mass=object.mass;entity->useGravity=object.gravity;
        entity->setLocalAABB(mesh.bounds);entity->setTexture(texture);
    }
    return result;
}
void Editor::flushHistory() {
    if(play.isPlaying())return;
    auto document=EditorFiles::capture(*scene);dirty=history.isDirty(document);
    history.commit(std::move(document),selected);
}
void Editor::restoreHistory(bool redo) {
    if(play.isPlaying())return;
    flushHistory();
    const auto* snapshot=redo ? history.redo() : history.undo();
    if(!snapshot)return;
    try {
        auto replacement=makeScene(snapshot->document,std::make_unique<EditorCamera>(*static_cast<EditorCamera*>(scene->getCamera())));
        replacement->preserveEntityIdsFrom(*scene);
        scene=std::move(replacement);selected=scene->findEntity(snapshot->selected) ? snapshot->selected : 0;
        dirty=history.isDirty(snapshot->document);status=redo ? "Redone scene edit" : "Undone scene edit";
    } catch(const std::exception& error) {
        if(redo)history.undo();else history.redo();
        status="Cannot restore scene edit: "+std::string(error.what());
    }
}
void Editor::viewportTools(glm::vec2 origin,glm::vec2 size,bool hovered) {
    auto& io=ImGui::GetIO();
    auto* camera=displayedScene().getCamera();
    auto view=camera->calcViewMatrix(),projection=camera->calcProjectionMatrix(static_cast<float>(width)/height);
    const bool allowed=!play.isPlaying()&&!navigating&&pending==Action::None&&fileMode==FileMode::None&&!ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopupId);
    if(allowed&&hovered&&!io.WantTextInput&&!ImGui::IsAnyItemActive()&&!io.KeyCtrl&&!io.KeyAlt) {
        if(ImGui::IsKeyPressed(ImGuiKey_W))transformTool=TransformTool::Move;
        if(ImGui::IsKeyPressed(ImGuiKey_E))transformTool=TransformTool::Rotate;
        if(ImGui::IsKeyPressed(ImGuiKey_R))transformTool=TransformTool::Scale;
        if(ImGui::IsKeyPressed(ImGuiKey_F))frameSelected();
    }
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());ImGuizmo::SetRect(origin.x,origin.y,size.x,size.y);
    ImGuizmo::SetOrthographic(false);ImGuizmo::Enable(allowed);
    auto* entity=displayedScene().findEntity(selected);
    if(entity && allowed) {
        auto matrix=entity->getModel();
        const auto operation=transformTool==TransformTool::Move ? ImGuizmo::TRANSLATE : transformTool==TransformTool::Rotate ? ImGuizmo::ROTATE : ImGuizmo::SCALE;
        auto mode=(localSpace||transformTool==TransformTool::Scale) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        if(ImGuizmo::Manipulate(glm::value_ptr(view),glm::value_ptr(projection),operation,mode,glm::value_ptr(matrix)))
            dirty |= EditorMath::applyTransform(*entity,matrix);
    }
    gizmoUsing=allowed&&entity&&ImGuizmo::IsUsing();
    if(allowed&&hovered&&!gizmoUsing&&(!entity||!ImGuizmo::IsOver())&&ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        auto ray=EditorMath::viewportRay({io.MousePos.x,io.MousePos.y},origin,size,view,projection);
        flushHistory();selected=ray ? EditorMath::pick(*scene,*ray) : 0;
        entity=scene->findEntity(selected);
    }
    if(entity) {
        auto* draw=ImGui::GetWindowDrawList();
        draw->PushClipRect({origin.x,origin.y},{origin.x+size.x,origin.y+size.y},true);
        const auto bounds=entity->getLocalAABB();const auto matrix=projection*view*entity->getModel();
        glm::vec4 points[8];
        for(int i=0;i<8;++i)points[i]=matrix*glm::vec4((i&1)?bounds.max.x:bounds.min.x,(i&2)?bounds.max.y:bounds.min.y,(i&4)?bounds.max.z:bounds.min.z,1);
        for(int i=0;i<8;++i)for(int bit:{1,2,4})if(!(i&bit)) {
            const auto a=points[i],b=points[i|bit];if(a.w<=0.001f||b.w<=0.001f)continue;
            auto screen=[&](glm::vec4 p){return ImVec2(origin.x+(p.x/p.w+1)*size.x*0.5f,origin.y+(1-p.y/p.w)*size.y*0.5f);};
            draw->AddLine(screen(a),screen(b),IM_COL32(255,198,80,255),1.5f);
        }
        draw->PopClipRect();
    }
}

void Editor::refreshAssets(){try{assetEntries=assets->list();if(!assetSelected.empty()&&std::none_of(assetEntries.begin(),assetEntries.end(),[&](const auto& entry){return entry.reference==assetSelected;}))assetSelected.clear();}catch(const std::exception& error){assetMessage=error.what();}}
void Editor::importAssets(const std::vector<std::filesystem::path>& files){
    if(play.isPlaying()){assetMessage="Stop Play before importing assets.";return;}
    std::size_t imported=0;std::string failures;
    for(const auto& file:files)try{assetSelected=assets->importFile(file);++imported;}catch(const std::exception& error){if(failures.size()<2048)failures+=AssetPaths::text(file.filename())+": "+error.what()+"\n";}
    refreshAssets();assetMessage="Imported "+std::to_string(imported)+" file(s). "+failures;
}
void Editor::assignAsset(const std::string& reference,Core::EntityId target){
    if(play.isPlaying())return;
    auto* entity=scene->findEntity(target);if(!entity)throw std::runtime_error("Select an object first, or drop onto one in the viewport.");
    const auto type=AssetLibrary::kind(reference);
    if(type==AssetKind::Texture){auto texture=assets->texture(reference);flushHistory();entity->setTexture(texture);entity->materialAsset=reference;}
    else if(type==AssetKind::Script){assets->scriptSource(reference);flushHistory();entity->scriptAsset=reference;}
    else throw std::runtime_error("Use Create object for a model asset.");
    selected=target;dirty=true;assetMessage="Assigned "+reference;
}
void Editor::createModel(const std::string& reference){
    if(play.isPlaying())return;
    auto loaded=assets->mesh(reference);auto texture=assets->texture("builtin/white");auto shader=assets->shader();flushHistory();
    auto* entity=scene->createEntity(loaded.mesh,shader);entity->meshAsset=reference;entity->materialAsset="builtin/white";
    entity->name=AssetPaths::text(AssetPaths::path(reference).stem());entity->setLocalAABB(loaded.bounds);entity->setTexture(texture);
    selected=entity->id();dirty=true;frameSelected();
}
void Editor::frameSelected(){if(auto* entity=displayedScene().findEntity(selected))static_cast<EditorCamera*>(displayedScene().getCamera())->focus(entity->getWorldAABB());}
void Editor::editScript(const std::string& reference){
    if(play.isPlaying())return;
    try{scriptText=assets->scriptSource(reference);savedScriptText=scriptText;editingScript=reference;scriptError.clear();showScript=true;}
    catch(const std::exception& error){assetMessage=error.what();}
}
void Editor::drawMaterialAndScript(Core::Entity& entity){
    ImGui::SeparatorText("Material");
    if(ImGui::BeginCombo("Texture",entity.materialAsset.c_str())){
        for(const auto& value:{std::string("builtin/white"),std::string("builtin/brick")})if(ImGui::Selectable(value.c_str(),entity.materialAsset==value)){
            try{auto texture=assets->texture(value);flushHistory();entity.setTexture(texture);entity.materialAsset=value;dirty=true;}catch(const std::exception& error){assetMessage=error.what();}
        }
        for(const auto& entry:assetEntries)if(entry.kind==AssetKind::Texture)if(ImGui::Selectable(entry.name.c_str(),entity.materialAsset==entry.reference))try{assignAsset(entry.reference,entity.id());}catch(const std::exception& error){assetMessage=error.what();}
        ImGui::EndCombo();
    }
    auto oldTint=entity.tint;
    if(ImGui::ColorEdit3("Tint",&entity.tint.x)){
        for(int i=0;i<3;++i)entity.tint[i]=std::isfinite(entity.tint[i])?std::clamp(entity.tint[i],0.0f,1.0f):oldTint[i];dirty|=entity.tint!=oldTint;
    }
    editFinished|=ImGui::IsItemDeactivatedAfterEdit();
    auto previous=entity.tiling;
    if(ImGui::DragFloat2("UV tiling",&entity.tiling.x,0.05f,0.01f,100,"%.2f",ImGuiSliderFlags_AlwaysClamp)){
        for(int i=0;i<2;++i)if(!std::isfinite(entity.tiling[i]))entity.tiling[i]=previous[i];dirty|=entity.tiling!=previous;
    }
    editFinished|=ImGui::IsItemDeactivatedAfterEdit();
    if(!entity.getTextures().empty())ImGui::Image(static_cast<ImTextureID>(entity.getTextures()[0]->ID),{64,64},{0,1},{1,0});
    ImGui::SeparatorText("Script");
    if(ImGui::BeginCombo("Lua script",entity.scriptAsset.empty()?"None":entity.scriptAsset.c_str())){
        if(ImGui::Selectable("None",entity.scriptAsset.empty())){flushHistory();entity.scriptAsset.clear();dirty=true;}
        for(const auto& entry:assetEntries)if(entry.kind==AssetKind::Script)if(ImGui::Selectable(entry.name.c_str(),entity.scriptAsset==entry.reference))try{assignAsset(entry.reference,entity.id());}catch(const std::exception& error){assetMessage=error.what();}
        ImGui::EndCombo();
    }
    if(ImGui::Button("New script"))try{auto reference=assets->createScript();refreshAssets();assignAsset(reference,entity.id());editScript(reference);}catch(const std::exception& error){assetMessage=error.what();}
    if(!entity.scriptAsset.empty()){ImGui::SameLine();if(ImGui::Button("Edit script"))editScript(entity.scriptAsset);}
    ImGui::TextWrapped("Scripts run in Play. Save script edits, then restart Play to reload them.");
}
void Editor::drawAssets(){
    ImGui::Begin("Assets");
    ImGui::BeginDisabled(play.isPlaying());
#ifdef _WIN32
    if(ImGui::Button("Import..."))importAssets(chooseAssetFiles(window->getWindow()));
    ImGui::SameLine();
#endif
    if(ImGui::Button("Import path...")){showImport=true;importPath.clear();}
    ImGui::SameLine();if(ImGui::Button("New script"))try{assetSelected=assets->createScript();refreshAssets();editScript(assetSelected);}catch(const std::exception& error){assetMessage=error.what();}
    ImGui::SameLine();if(ImGui::Button("Rescan"))refreshAssets();
    ImGui::SameLine();ImGui::SetNextItemWidth(140);ImGui::InputTextWithHint("##AssetFilter","Find asset...",&assetFilter);
    if(ImGui::BeginTable("AssetBrowser",2,ImGuiTableFlags_Resizable|ImGuiTableFlags_BordersInnerV)){
        ImGui::TableSetupColumn("Files",ImGuiTableColumnFlags_WidthStretch,2);ImGui::TableSetupColumn("Selected",ImGuiTableColumnFlags_WidthStretch,1);
        ImGui::TableNextRow();ImGui::TableNextColumn();
        for(const auto& entry:assetEntries)if(assetFilter.empty()||entry.name.find(assetFilter)!=std::string::npos){
            const auto label=std::string(entry.kind==AssetKind::Model?"[OBJ] ":entry.kind==AssetKind::Texture?"[IMG] ":"[LUA] ")+entry.name;
            if(ImGui::Selectable(label.c_str(),assetSelected==entry.reference))assetSelected=entry.reference;
            if(ImGui::BeginDragDropSource()){ImGui::SetDragDropPayload("FERSA_ASSET",entry.reference.c_str(),entry.reference.size()+1);ImGui::TextUnformatted(entry.name.c_str());ImGui::EndDragDropSource();}
        }
        if(assetEntries.empty())ImGui::TextWrapped("Drop OBJ, PNG/JPEG, or Lua files from your file manager into this window.");
        ImGui::TableNextColumn();
        if(!assetSelected.empty())try{
            const auto type=AssetLibrary::kind(assetSelected);ImGui::TextWrapped("%s",AssetPaths::text(AssetPaths::path(assetSelected).filename()).c_str());
            if(type==AssetKind::Model){if(ImGui::Button("Create object"))createModel(assetSelected);}
            else {ImGui::BeginDisabled(!scene->findEntity(selected));if(ImGui::Button(type==AssetKind::Texture?"Apply texture":"Attach script"))assignAsset(assetSelected,selected);ImGui::EndDisabled();}
            if(type==AssetKind::Script&&ImGui::Button("Edit source"))editScript(assetSelected);
            if(type==AssetKind::Texture){auto texture=assets->texture(assetSelected);ImGui::Image(static_cast<ImTextureID>(texture->ID),{48,48},{0,1},{1,0});}
        }catch(const std::exception& error){assetMessage=error.what();}
        ImGui::EndTable();
    }
    ImGui::EndDisabled();
    ImGui::TextDisabled("Project: %s",AssetPaths::text(assets->root().filename()).c_str());
    if(ImGui::IsItemHovered())ImGui::SetTooltip("%s",AssetPaths::text(assets->root()).c_str());
    if(!assetMessage.empty())ImGui::TextWrapped("%s",assetMessage.c_str());
    if(!play.diagnostics().empty()&&ImGui::CollapsingHeader("Script console",ImGuiTreeNodeFlags_DefaultOpen))for(const auto& message:play.diagnostics())ImGui::TextWrapped("%s",message.c_str());
    ImGui::End();
    if(showImport){ImGui::OpenPopup("Import file");showImport=false;}
    if(ImGui::BeginPopupModal("Import file",nullptr,ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::TextUnformatted("Choose an OBJ, PNG/JPEG, or Lua source file.");ImGui::SetNextItemWidth(600);ImGui::InputText("Path",&importPath);
        if(ImGui::Button("Import")){importAssets({AssetPaths::path(importPath)});ImGui::CloseCurrentPopup();}ImGui::SameLine();if(ImGui::Button("Cancel"))ImGui::CloseCurrentPopup();ImGui::EndPopup();
    }
    if(showScript){ImGui::OpenPopup("Script editor");showScript=false;}
    ImGui::SetNextWindowSize({800,580},ImGuiCond_Appearing);
    if(ImGui::BeginPopupModal("Script editor",nullptr)){
        ImGui::TextWrapped("%s%s",editingScript.c_str(),scriptText!=savedScriptText?" *":"");
        ImGui::TextWrapped("return { on_start = function(self) ... end, on_update = function(self, dt) ... end }");
        ImGui::InputTextMultiline("##LuaSource",&scriptText,{-1,-110},ImGuiInputTextFlags_AllowTabInput);
        if(!scriptError.empty())ImGui::TextWrapped("%s",scriptError.c_str());
        auto save=[&](){try{assets->saveScript(editingScript,scriptText);savedScriptText=scriptText;scriptError="Saved. Restart Play to reload.";return true;}catch(const std::exception& error){scriptError=error.what();return false;}};
        if(ImGui::Button("Save script") || (ImGui::GetIO().KeyCtrl&&ImGui::IsKeyPressed(ImGuiKey_S,false)))save();ImGui::SameLine();
        if(ImGui::Button("Close")){if(scriptText!=savedScriptText)ImGui::OpenPopup("Unsaved script edits");else{editingScript.clear();ImGui::CloseCurrentPopup();}}
        if(ImGui::BeginPopupModal("Unsaved script edits",nullptr,ImGuiWindowFlags_AlwaysAutoResize)){
            ImGui::TextUnformatted("Save script changes before closing?");
            bool close=false;
            if(ImGui::Button("Save and close")){if(save())close=true;}ImGui::SameLine();
            if(ImGui::Button("Discard edits"))close=true;ImGui::SameLine();if(ImGui::Button("Cancel"))ImGui::CloseCurrentPopup();
            if(close){editingScript.clear();scriptText=savedScriptText;ImGui::CloseCurrentPopup();}
            ImGui::EndPopup();
        }
        if(editingScript.empty())ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
