#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Core::Render {
class Shader {
public:
    unsigned int ID = 0;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Allow moving
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void activate() const;
    void del() const;

    void setBol(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;

    void setMat3(const std::string& name, const glm::mat3& matrix) const;
    void setMat4(const std::string& name, const glm::mat4& matrix) const;
};
}

