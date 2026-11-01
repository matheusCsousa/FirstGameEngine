#pragma once
#include "../core/Scene/Scene.hpp"
#include "ScriptRuntime.hpp"

// Owns only the disposable runtime scene. Authored data and document dirty state stay with Editor.
class PlaySession {
    std::unique_ptr<Core::Scene> runtime;
    ScriptRuntime scripts;
    double accumulator = 0;
    bool paused = false;
    std::size_t steps = 0;
    void tick();
public:
    static constexpr double fixedStep = 1.0 / 120.0;
    static constexpr int maxStepsPerFrame = 8;
    void start(const Core::Scene& authored, std::unique_ptr<Core::Graphics::Camera> camera,const std::filesystem::path& projectRoot={});
    void stop();
    void advance(double frameSeconds);
    void setPaused(bool value);
    void step();
    bool isPlaying() const { return runtime != nullptr; }
    bool isPaused() const { return paused; }
    std::size_t stepCount() const { return steps; }
    const std::vector<std::string>& diagnostics() const { return scripts.diagnostics(); }
    Core::Scene* scene() { return runtime.get(); }
};
