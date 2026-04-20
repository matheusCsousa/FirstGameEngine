#pragma once

#include <vector>
#include <memory>

#include "../Entity/Entity.hpp"
#include "../Camera/Camera.hpp"

namespace Core {
struct Window;
class Scene {
private:
    std::unique_ptr<Core::Graphics::Camera> m_camera;
    std::vector<std::unique_ptr<Core::Entity>> m_entities;

public:
    Scene(std::unique_ptr<Core::Graphics::Camera> camera);
    ~Scene() = default;

    Core::Entity* createEntity( Core::Graphics::Mesh mesh, glm::mat4 model, Core::Render::Shader shader);
    std::vector<std::unique_ptr<Core::Entity>>& getEntities();

    void render(Window* window);

    Core::Graphics::Camera* getCamera();
};
}
