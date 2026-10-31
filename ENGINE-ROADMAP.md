# FirstGameEngine improvement roadmap

This roadmap is based on a source review of the current checkout. These findings describe the baseline before the runtime-foundation changes; see README.md for the current build and test workflow.

## First milestone: reliable startup, shutdown, and execution

- Release application logic, scenes, and GPU resources while the OpenGL context is still current. `core/Game/Game.cpp` currently destroys the window in the destructor body before `m_logic` is destroyed.
- Propagate GLFW/window/GLEW initialization failure to the application. `Window::create()` currently returns after logging failures, while the Game constructor continues initializing input. After GLEW failure it also leaves the destroyed native window pointer non-null.
- Reject `Game::run()` without registered logic instead of dereferencing a null `m_logic`.
- Use the configured game title and initialize framebuffer dimensions from GLFW. Handle zero-sized framebuffers while minimized before computing the aspect ratio.
- Make explicit GPU deletion methods consistent with RAII: Shader/VAO/VBO/EBO `del()` currently leave their handles live for later destructor deletion.
- Establish a reproducible CMake dependency setup for GLFW, GLEW, GLM, and OpenGL; provide Windows/Linux build instructions and CI. The README mentions bundled static libraries while the graphics CMake target uses package discovery.
- Resolve and copy runtime assets independently of the caller's working directory.

Acceptance: clean-checkout build; launch from another working directory; useful error for missing assets; clean shutdown; initialization failure cannot reach the update/render loop.

## Second milestone: predictable input and simulation

- Move frame timing into the game loop, initialize its clock after setup, and expose time explicitly to update functions.
- Introduce fixed-step physics with a bounded frame delta and catch-up limit. Physics currently integrates the full variable frame delta taken from Input.
- Correct collision-axis selection for equal penetration depths in `core/Physics/Physics.cpp`. For overlap depths `(1, 1, 5)`, the strict comparisons choose Z with penetration 5, instead of an axis with penetration 1.
- Add focused tests for separated/touching/overlapping AABBs, equal-depth ties, dynamic/static collisions, transformed bounds, and long frame pauses.
- Accumulate mouse movement across events in a frame instead of keeping only the last movement event.
- Distinguish key repeat from a fresh press, validate mouse-button indices, and reset input on focus loss.

Acceptance: stable falling-body/platform demo under varying render frame rates; one-shot controls fire once per actual press; invalid input indices are harmless.

## Third milestone: assets and rendering

- Report shader file/compile/link failures as failed asset loads; callers must not continue rendering an invalid program.
- Validate OBJ faces before triangulation. An empty face underflows `faceTokens.size() - 1`; invalid numeric tokens throw; negative OBJ indices are not currently resolved.
- Define a Vertex structure instead of repeating the 11-float layout in loading and rendering code.
- Handle all supported texture channel counts and pixel-unpack alignment deliberately. Current two-channel images fall through to RGB, and RGB row alignment is not configured.
- Represent materials and scene lights as data. `Scene::render()` currently hardcodes a single light's position and color.
- Compute per-frame camera matrices once; cache shader uniform locations. Profile before adding batching or instancing.
- Add a small diagnostics overlay for frame time, entity count, draw calls, and collision bounds.

Acceptance: invalid models/textures/shaders fail clearly; a scene contains several independently configured materials and lights; rendering diagnostics are visible.

## Fourth milestone: prove the engine with a small game

Build one complete sample: a controllable character or camera, a small level, collisions, an interaction/objective, and reset/restart. Let its actual needs guide subsequent reusable APIs.

Then prioritize:

- Stable entity identifiers and safe entity removal.
- Separating transform, render, and physics state when the sample requires it.
- Scene save/load and configurable input bindings.
- Asset reuse and scene transitions.
- Debug controls, audio, and basic UI.
- A visual editor after scene data and runtime ownership are established.

## Suggested first implementation PR

Keep the first PR bounded to startup/shutdown ownership and failure handling, asset-path setup, and build verification. Follow with simulation/input fixes and their focused tests. Each milestone should leave the sample runnable.
