#pragma once
#include "../core/Logic/Logic.hpp"
#include "../core/Scene/Scene.hpp"
#include "../core/Window/Window.hpp"
#include <filesystem>
#include "SceneDocument.hpp"
#include "PlaySession.hpp"

class Editor final : public Core::Logic {
    std::shared_ptr<Core::Window> window;
    std::unique_ptr<Core::Scene> scene;
    std::shared_ptr<Core::Graphics::Mesh> cubeMesh;
    std::shared_ptr<Core::Render::Shader> cubeShader;
    std::shared_ptr<Core::Render::Texture> cubeTexture;
    Core::Collision::AABB cubeBounds;
    PlaySession play;
    void startPlay();
    void stopPlay();
    Core::Scene& displayedScene() { return play.isPlaying() ? *play.scene() : *scene; }
    Core::EntityId selected = 0;
    unsigned framebuffer = 0, color = 0, depth = 0;
    int width = 0, height = 0;
    bool navigating = false;
    bool dirty = false;
    enum class Action { None, New, Open, Close };
    enum class FileMode { None, Open, Save };
    Action pending = Action::None;
    FileMode fileMode = FileMode::None;
    bool showUnsaved = false, showFile = false, replaceExisting = false;
    std::filesystem::path currentPath;
    std::string pathInput, fileError, status;
    void request(Action action);
    void executePending();
    void openFileDialog(FileMode mode);
    bool saveTo(const std::filesystem::path& path);
    bool loadFrom(const std::filesystem::path& path);
    void fileControls();
    void resizeViewport(int w, int h);
    void releaseNavigation();
    void createCube();
public:
    Editor(std::shared_ptr<Core::Window> window, const std::filesystem::path& assets);
    ~Editor() override;
    bool onCloseRequested() override;
    void onUpdate() override;
    void onRender() override;
};
