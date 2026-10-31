# FirstGameEngine

A modular 3D game engine built from scratch in **C++** using **OpenGL**, **GLFW**, and **GLEW**.

The engine is designed with modularity in mind, separating reusable engine code from game-specific logic. Each engine subsystem is compiled as its own static library, making the project easy to extend and maintain.

![Engine Screenshot](./.github/images/cubogirando.png)

---

## Features

- Modern OpenGL rendering pipeline
- Modular engine architecture
- Scene and entity system
- First-person free camera
- Shader abstraction
- Texture loading
- OBJ mesh support
- Asset management system
- Keyboard and mouse input handling
- Delta-time movement
- Physics module *(work in progress)*
- AABB collision primitives
- CMake build system
- Static library separation for engine modules

---

## Project Structure

```text
FirstGameEngine/
├── core/
│   ├── AssetManager/
│   ├── Collision/
│   ├── Entity/
│   ├── Event/
│   ├── Game/
│   ├── Graphics/
│   │   ├── Camera/
│   │   ├── Mesh/
│   │   ├── Renderer/
│   │   │   ├── Shader/
│   │   │   ├── Texture/
│   │   │   ├── VAO/
│   │   │   ├── VBO/
│   │   │   └── EBO/
│   │   └── lib/
│   ├── Input/
│   ├── Logic/
│   ├── Physics/
│   ├── Scene/
│   └── Window/
│
├── gameApp/
│   ├── Camera/
│   ├── models/
│   ├── shader/
│   ├── textures/
│   ├── GameLogic.cpp
│   └── main.cpp
│
├── CMakeLists.txt
└── README.md
```

---

## Architecture

The engine is composed of independent modules that communicate through well-defined interfaces.

```
Application
      │
      ▼
    Game Loop
      │
 ┌────┴──────────────────────────┐
 │                               │
 ▼                               ▼
Input                         Scene
 │                               │
 ▼                               ▼
Events                      Entities
 │                               │
 └──────────────┬────────────────┘
                ▼
            Renderer
      ├── Shader
      ├── Texture
      ├── Mesh
      ├── VAO
      ├── VBO
      └── EBO
                │
                ▼
             OpenGL
```

Game-specific behavior is implemented by inheriting from the abstract `Logic` interface.

```cpp
class Logic
{
public:
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
};
```

The engine owns the game loop while the application provides the gameplay implementation.

---

## Rendering

The renderer currently provides abstractions for:

- Vertex Array Objects (VAO)
- Vertex Buffer Objects (VBO)
- Element Buffer Objects (EBO)
- GLSL shader management
- Texture loading
- Mesh rendering
- View and projection matrices

---

## Camera

The sample application includes a free-fly camera.

| Input | Action |
|-------|--------|
| **W / S** | Move forward / backward |
| **A / D** | Strafe left / right |
| **Q / E** | Move down / up |
| **Mouse** | Look around |
| **F** | Toggle cursor capture |
| **ESC** | Exit application |

---

## Asset Management

The engine includes an AssetManager responsible for loading and organizing assets.

Currently supported:

- GLSL shaders
- Textures
- OBJ models

---

## Physics

A dedicated physics module exists for future expansion.

Current functionality includes:

- Physics module
- AABB collision primitives

---

## Input System

The input system tracks multiple key states.

```cpp
enum KeyState
{
    KEY_RELEASED,
    KEY_PRESSED,
    KEY_HELD
};
```

Example usage:

```cpp
Input::isKeyPressed(GLFW_KEY_W);
Input::isKeyHeld(GLFW_KEY_W);

float deltaTime = Input::getDeltaTime();
```

---

## Dependencies

- OpenGL
- GLFW
- GLEW
- GLM
- stb_image
- CMake 3.24 or newer
- A C++23 compiler (MSVC, GCC, or Clang)

CMake fetches versioned GLFW 3.4, GLEW 2.2.0, and GLM 1.0.1 sources by default and builds them with the selected compiler. The pre-existing archives in `core/Graphics/lib/` are not used. The first configure requires network access.

To use installed packages instead, configure with `-DFERSA_FETCH_DEPENDENCIES=OFF`; packages must provide `glfw`, `GLEW::GLEW`, and `glm::glm` targets.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
```

On Linux, install OpenGL/X11 development packages first. For Ubuntu:

```sh
sudo apt-get install cmake ninja-build libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

On Windows, use a developer terminal with Visual Studio C++ build tools, or configure a suitable CMake toolchain. Dependencies are compiled from source; no manually copied `.a` files are required.

## Running

Single-configuration builds (Linux, Ninja):

```sh
./build/gameApp/fersa
```

Visual Studio builds:

```powershell
.\build\gameApp\Debug\fersa.exe
```

Assets are copied into `assets/` beside the executable after the sample is linked. Launching via the executable path above also works from another working directory. A launcher that only supplies a bare executable name via PATH can explicitly set the asset location:

```sh
fersa --assets /path/to/assets
```

`--smoke-test` creates a hidden OpenGL window, renders three frames, and exits. Startup errors are printed to stderr and return a nonzero exit status.

## Runtime checks

```sh
ctest --test-dir build -C Debug --output-on-failure
```

Graphics tests require a working OpenGL 3.3 context. Linux CI runs them under Xvfb/Mesa:

```sh
xvfb-run -a ctest --test-dir build --output-on-failure
```

Checks cover initialization validation, multiple-window GLFW lifetime, running without application logic, bounded updates/rendering, releasing application logic with its context still current, idempotent GPU deletion, mesh index-buffer ownership, sample asset resolution from another directory, missing assets, and Linux startup without a display. Windows CI builds the engine and runs the display-independent missing-assets, scene-document, transform-control, play-session, and viewport-authoring checks; run the graphics checks locally with an OpenGL-capable driver.

## Editor direction

Builds now include **fersa_editor**, a separate Dear ImGui docking application:

```sh
# Windows multi-configuration build
./build/editor/Debug/fersa_editor.exe
# Linux / Ninja single-configuration build
./build/editor/fersa_editor
```

The editor opens a cube and ground scene. Select objects in Hierarchy; edit names, positions, rotations (degrees), and positive scales in Inspector. Create cubes, duplicate, reset transforms, and delete objects. The Scene panel renders to a resizing framebuffer. Hold the right mouse button over it to fly with WASD, Q/E for vertical movement, and Shift for speed. Release RMB or press Escape to release the camera.

Use **File > Save / Save as** (Ctrl+S / Ctrl+Shift+S) to write a scene; **Open** (Ctrl+O) restores it, and **New** (Ctrl+N) starts empty. The path dialog accepts a full path; its folder must already exist. Save as requires explicit replacement permission for an existing different file. An asterisk marks unsaved edits. New, Open, Exit, and the native close button offer Save/Discard/Cancel when needed. Failed saves keep the dirty state; failed opens preserve the current scene.

Scene files use versioned JSON (`.fersa.json`) with IDs, names, transforms, asset references, and basic physics state. Version 1 supports the editor's cube/brick preset; unsupported asset references and malformed documents are rejected. Saves write a sibling temporary file before replacing the destination. See [SCENE-FORMAT.md](SCENE-FORMAT.md).

Drag transform fields horizontally or Ctrl+click to enter a number. Position and rotation accept negative values; scale stays between 0.01 and 1000. Physics remains paused in Edit mode. Set an object's mass above zero and enable gravity in Inspector, then press **Play** in the top bar. The initial sample already contains a falling cube and static ground. **Pause/Resume** freezes/resumes preview; **Step** advances one 1/120-second physics tick while paused. **Stop** discards the runtime copy and restores authored transforms, velocities, and camera state. The preview does not mark the document dirty or change saved files.

Object and physics editing, New/Open, and saving are disabled during preview. Close/Exit stops preview first, then protects any unsaved authored changes. You can still select objects and navigate the runtime camera. The inspector displays runtime values read-only; stop to change mass, gravity, or initial velocity.

Preview physics uses a 120 Hz fixed timestep with at most eight catch-up ticks per frame. Large frame delays are discarded rather than causing a large simulation jump. This applies to the editor preview; the legacy runtime sample still uses its existing variable update. Collisions use the existing simple world-AABB solver, with corrected equal-penetration axis selection. Click a cube in the viewport to select it (nearest transformed cube bounds); click empty space to deselect. A gold bounds outline marks the selection. Choose **Move / Rotate / Scale** above the viewport, or use **W / E / R** while the pointer is over the viewport. Drag the colored handles to edit the object. Move and Rotate support local/world axes; Scale always uses local axes and stays positive. Tool shortcuts do not run while typing, navigating with RMB, or previewing physics. Picking and gizmos are disabled during Play; hierarchy selection remains available for inspection.

**Edit > Undo/Redo** or **Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z** restores scene edits, including create/duplicate/delete, names, transforms, resets, mass, gravity, and initial velocity. A complete drag or text edit is one history action. Selection and camera movement are not undo actions. Text fields retain their own native undo while active. Undo/redo is disabled during Play. Returning to saved scene content clears the dirty marker; branching after Undo discards the old redo branch. New/Open reset history, while Save preserves it. History is session-only and keeps up to 100 snapshots within an approximate 32 MiB budget, in addition to the saved checkpoint.

Scripts and materials/assets UI remain future milestones. Picking currently uses transformed cube bounds, not arbitrary triangle meshes. Gizmos reject reflected, sheared, non-finite, or out-of-range transforms. Dock layouts reset on launch.

Dear ImGui is pinned to `v1.92.0-docking`. An offline checkout can be supplied with `-DFERSA_IMGUI_SOURCE_DIR=/path/to/imgui`; use `-DFERSA_BUILD_EDITOR=OFF` to build only the runtime/sample. `FERSA_FETCH_DEPENDENCIES=OFF` controls the graphics libraries; the editor still needs ImGui, ImGuizmo, and nlohmann/json source. ImGuizmo is pinned to commit `105e4fde45e4e74aae8973d98750820ef7aca7e2`; an offline source tree can be supplied through `FETCHCONTENT_SOURCE_DIR_IMGUIZMO`. JSON is pinned to v3.11.3; an offline source tree can be supplied with CMake's `FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON`. The editor also accepts `--assets DIRECTORY` and `--smoke-test`, and receives its own copied assets next to the executable.

Tests include stable/restored IDs, duplicate independence, deletion lookup, close cancellation, play-copy isolation, pause/resume/step, fixed-step frame-rate consistency, collision ties/ground landing, real ImGui transform drags, viewport rays and rotated-bound picking, gizmo XYZ matrix round trips, bounded undo/redo branches and saved checkpoints, scene file round trips and invalid-input handling, and a hidden editor startup/render/shutdown check. See [EDITOR-MVP.md](EDITOR-MVP.md) for the remaining authoring workflow and [ENGINE-ROADMAP.md](ENGINE-ROADMAP.md) for the original engine audit.

---

## Creating a Game

Create a class derived from `Core::Logic`.

```cpp
class MyGame : public Core::Logic
{
public:
    void onUpdate() override;
    void onRender() override;
};
```

Then register it with the engine.

```cpp
#include "core/Game/Game.hpp"
#include "MyGame.hpp"

int main()
{
    Core::GameSpecs specs;

    specs.title = "My Game";
    specs.windowSpec.width = 1280;
    specs.windowSpec.height = 720;

    Core::Game game(specs);

    game.pushLogic<MyGame>();

    game.run();
}
```
