#pragma once
#include "../core/Logic/Logic.hpp"
#include "../core/Scene/Scene.hpp"
#include "../core/Window/Window.hpp"
#include <filesystem>
#include "SceneDocument.hpp"
#include "PlaySession.hpp"
#include "EditHistory.hpp"
#include "AssetLibrary.hpp"

class Editor final : public Core::Logic {
    std::shared_ptr<Core::Window> window;
    std::unique_ptr<Core::Scene> scene;
    std::shared_ptr<Core::Graphics::Mesh> cubeMesh;
    std::shared_ptr<Core::Render::Shader> cubeShader;
    std::shared_ptr<Core::Render::Texture> cubeTexture;
    Core::Collision::AABB cubeBounds;
    std::unique_ptr<AssetLibrary> assets;
    std::vector<AssetEntry> assetEntries;
    std::vector<std::filesystem::path> pendingImports;
    void (*previousDrop)(GLFWwindow*,int,const char**)=nullptr;
    std::string assetSelected,assetFilter,assetMessage,importPath;
    bool showImport=false,showScript=false;
    std::string editingScript,scriptText,savedScriptText,scriptError;
    void refreshAssets();
    void importAssets(const std::vector<std::filesystem::path>& files);
    void drawAssets();
    void drawMaterialAndScript(Core::Entity& entity);
    void assignAsset(const std::string& reference,Core::EntityId target);
    void createModel(const std::string& reference);
    void editScript(const std::string& reference);
    void frameSelected();
    EditHistory history;
    bool gizmoUsing=false, editFinished=false;
    enum class TransformTool { Move, Rotate, Scale };
    TransformTool transformTool=TransformTool::Move;
    bool localSpace=true;
    void flushHistory();
    void restoreHistory(bool redo);
    void viewportTools(glm::vec2 origin,glm::vec2 size,bool hovered);
    std::unique_ptr<Core::Scene> makeScene(const EditorFiles::Document& document,std::unique_ptr<Core::Graphics::Camera> camera,AssetLibrary* library=nullptr);
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
    void drawCreateMenu();
    std::string hierarchyFilter;
    bool resetLayout=false;
public:
    Editor(std::shared_ptr<Core::Window> window, const std::filesystem::path& assets,const std::filesystem::path& project={});
    ~Editor() override;
    bool onCloseRequested() override;
    void onUpdate() override;
    void onRender() override;
};
