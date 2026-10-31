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

Checks cover initialization validation, multiple-window GLFW lifetime, running without application logic, bounded updates/rendering, releasing application logic with its context still current, idempotent GPU deletion, mesh index-buffer ownership, sample asset resolution from another directory, missing assets, and Linux startup without a display. Windows CI builds the engine and runs the display-independent missing-assets check; run the graphics checks locally with an OpenGL-capable driver.

## Editor direction

The next milestones are a separate editor application, scene hierarchy, transform inspector, viewport, and scene save/load. See [EDITOR-MVP.md](EDITOR-MVP.md) and [ENGINE-ROADMAP.md](ENGINE-ROADMAP.md).

The current executable remains a runtime sample. It does not yet include editor panels, scene documents, gizmos, or Play/Stop state management.

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
