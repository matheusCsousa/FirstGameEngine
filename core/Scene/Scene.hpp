#pragma once

#include <vector>
#include <memory>

#include "../Entity/Entity.hpp"
#include "../Camera/Camera.hpp"

namespace Core {
struct Window;
class Scene {
private:
    std::vector<std::unique_ptr<Core::Entity>> m_entities;
    Core::Graphics::Camera m_camera;

public:
    Scene(Core::Graphics::Camera camera);
    ~Scene() = default;

    Core::Entity* createEntity( Core::Graphics::Mesh mesh, glm::mat4 model, Core::Render::Shader shader);
    std::vector<std::unique_ptr<Core::Entity>>& getEntities();
    void render(Window& window);
};
}
