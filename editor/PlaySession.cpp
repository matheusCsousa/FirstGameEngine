#include "PlaySession.hpp"
#include "../core/Physics/Physics.hpp"
#include <algorithm>
#include <cmath>

void PlaySession::start(const Core::Scene& authored, std::unique_ptr<Core::Graphics::Camera> camera) {
    if (runtime) return;
    auto copy=authored.clone(std::move(camera));
    runtime=std::move(copy); accumulator=0; steps=0; paused=false;
}
void PlaySession::stop() { runtime.reset(); accumulator=0; steps=0; paused=false; }
void PlaySession::tick() {
    Core::Physics::PhysicsEngine::update(static_cast<float>(fixedStep),runtime->getEntities());
    ++steps;
}
void PlaySession::advance(double frameSeconds) {
    if (!runtime || paused || !std::isfinite(frameSeconds) || frameSeconds<=0) return;
    accumulator+=std::min(frameSeconds,fixedStep*maxStepsPerFrame);
    int count=0;
    while (accumulator+1e-10>=fixedStep && count<maxStepsPerFrame) {
        tick(); accumulator=std::max(0.0,accumulator-fixedStep); ++count;
    }
    // Long frames are deliberately bounded, rather than simulating a large accumulated backlog.
    if (accumulator>=fixedStep) accumulator=std::fmod(accumulator,fixedStep);
}
void PlaySession::setPaused(bool value) {
    if (!runtime) return;
    paused=value; accumulator=0;
}
void PlaySession::step() { if (runtime && paused) tick(); }
