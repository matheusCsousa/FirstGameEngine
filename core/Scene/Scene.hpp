#pragma once

#include "../Entity/Entity.hpp"
#include "../Camera/Camera.hpp"
#include "../AssetManager/AssetManager.hpp"

namespace Core {
struct Window;

class Scene {
private:
    std::unique_ptr<Core::Graphics::Camera> m_camera;
    std::vector<std::unique_ptr<Core::Entity>> m_entities;
    std::shared_ptr<Core::AssetManager> m_assetManager;

public:
    Scene(std::unique_ptr<Core::Graphics::Camera> camera);
    ~Scene() = default;

    Core::Entity* createEntity(std::shared_ptr<Core::Graphics::Mesh> mesh, std::shared_ptr<Core::Render::Shader> shader);
    std::vector<std::unique_ptr<Core::Entity>>& getEntities();

    std::shared_ptr<Core::AssetManager> getAssetManager() { return m_assetManager; }

    void render(Window* window);
    void update();

    Core::Graphics::Camera* getCamera();
};
}
