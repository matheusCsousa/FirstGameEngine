#include "Shader.hpp"

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

namespace Core::Render {

static std::string ReadFile(const char* path) {
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "Failed to open shader file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

static void CheckCompile(GLuint shader, const char* type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);

        std::cout << "Shader compilation error (" << type << ")\n"
                    << infoLog << std::endl;
    }
}

static void CheckLink(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);

        std::cout << "Shader linking error\n"
                    << infoLog << std::endl;
    }
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = ReadFile(vertexPath);
    std::string fragmentCode = ReadFile(fragmentPath);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    CheckCompile(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    CheckCompile(fragment, "FRAGMENT");

    ID = glCreateProgram();

    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    glLinkProgram(ID);
    CheckLink(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
    }
}

Shader::Shader(Shader&& other) noexcept : ID(other.ID) {
    other.ID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteProgram(ID);
        }
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

void Shader::activate() const {
    glUseProgram(ID);
}

void Shader::del() const {
    glDeleteProgram(ID);
}

void Shader::setBol(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        glm::value_ptr(value)
    );
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        glm::value_ptr(value)
    );
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        glm::value_ptr(value)
    );
}

void Shader::setMat3(const std::string& name, const glm::mat3& matrix) const {
    glUniformMatrix3fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(matrix)
    );
}

void Shader::setMat4(const std::string& name, const glm::mat4& matrix) const {
    glUniformMatrix4fv(
        glGetUniformLocation(ID, name.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(matrix)
    );
}

}
