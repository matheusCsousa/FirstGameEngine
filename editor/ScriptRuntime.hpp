#pragma once
#include "../core/Scene/Scene.hpp"
#include <filesystem>
#include <vector>
#include <string>
class ScriptRuntime {
    struct Instance;
    std::vector<std::unique_ptr<Instance>> instances;
    std::vector<std::string> messages;
public:
    ScriptRuntime();
    ~ScriptRuntime();
    static std::string checkSyntax(const std::string& source);
    void start(Core::Scene& scene,const std::filesystem::path& projectRoot);
    void update(Core::Scene& scene,float dt);
    void stop();
    const std::vector<std::string>& diagnostics() const { return messages; }
};
