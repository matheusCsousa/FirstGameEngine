#pragma once
#include "../core/Logic/Logic.hpp"
#include "../core/Scene/Scene.hpp"
#include "../core/Window/Window.hpp"
#include <filesystem>

class Editor final : public Core::Logic {
    std::shared_ptr<Core::Window> window;
    std::unique_ptr<Core::Scene> scene;
    std::shared_ptr<Core::Graphics::Mesh> cubeMesh;
    std::shared_ptr<Core::Render::Shader> cubeShader;
    std::shared_ptr<Core::Render::Texture> cubeTexture;
    Core::Collision::AABB cubeBounds;
    Core::EntityId selected = 0;
    unsigned framebuffer = 0, color = 0, depth = 0;
    int width = 0, height = 0;
    bool navigating = false;
    void resizeViewport(int w, int h);
    void releaseNavigation();
    void createCube();
public:
    Editor(std::shared_ptr<Core::Window> window, const std::filesystem::path& assets);
    ~Editor() override;
    void onUpdate() override {}
    void onRender() override;
};
