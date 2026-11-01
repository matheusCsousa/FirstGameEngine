#pragma once

#include "../Entity/Entity.hpp"
#include "../Graphics/Camera/Camera.hpp"
#include "../AssetManager/AssetManager.hpp"

namespace Core {
class Window;

class Scene {
private:
    EntityId m_nextId = 1;
    std::unique_ptr<Core::Graphics::Camera> m_camera;
    std::vector<std::unique_ptr<Core::Entity>> m_entities;
    std::shared_ptr<Core::AssetManager> m_assetManager;

public:
    Scene(std::unique_ptr<Core::Graphics::Camera> camera);
    ~Scene() = default;

    Core::Entity* createEntity(std::shared_ptr<Core::Graphics::Mesh> mesh, std::shared_ptr<Core::Render::Shader> shader, EntityId requestedId = 0);
    std::vector<std::unique_ptr<Core::Entity>>& getEntities();

    const std::vector<std::unique_ptr<Core::Entity>>& getEntities() const { return m_entities; }

    std::shared_ptr<Core::AssetManager> getAssetManager() { return m_assetManager; }

    EntityId nextEntityId() const { return m_nextId; }
    void preserveEntityIdsFrom(const Scene& other);
    Entity* findEntity(EntityId id);
    bool removeEntity(EntityId id);
    Entity* duplicateEntity(EntityId id);
    std::unique_ptr<Scene> clone(std::unique_ptr<Core::Graphics::Camera> camera) const;
    void render(float aspectRatio);
    void render(Window* window);
    void update();

    Core::Graphics::Camera* getCamera();
};
}
