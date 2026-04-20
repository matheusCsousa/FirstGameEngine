# FirstGameEngine

A 3D game engine built from scratch in C++ using OpenGL, GLFW and GLEW. The project is split into two layers: a reusable `core` engine library and a `gameApp` application that consumes it.

![screenshot](/.github/images/cubogirando.png)

## Features

- OpenGL rendering pipeline with VAO, VBO, EBO and shader management
- First-person free camera with mouse look and delta-time movement
- Scene and entity system with per-entity transform (position, rotation, scale)
- Input system with key states (pressed / held / released) and mouse tracking
- Abstract `Logic` interface to decouple game logic from the engine loop
- CMake build system with each module compiled as a static library

## Project Structure

```
game_engine/
├── core/                      # Engine — compiled as static libraries
│   ├── Game/                  # Game loop and entry point
│   ├── Window/                # GLFW window management
│   ├── Input/                 # Keyboard and mouse input
│   ├── Logic/                 # Abstract interface for game logic
│   ├── Scene/                 # Scene management (entities + camera)
│   ├── Entity/                # Base game object with transform
│   ├── Event/                 # Event system
│   └── Graphics/
│       ├── Camera/            # Base camera (view + projection matrices)
│       ├── Mesh/              # Mesh base class + Cube primitive
│       └── Renderer/
│           ├── Shader/        # GLSL shader compilation and uniforms
│           ├── VAO/           # Vertex Array Object
│           ├── VBO/           # Vertex Buffer Object
│           └── EBO/           # Element Buffer Object
└── gameApp/                   # Sample application — compiled as executable
    ├── main.cpp               # Entry point
    ├── GameLogic.cpp/.hpp     # Implements Core::Logic
    ├── Camera/freeCam.cpp     # First-person camera (extends Core::Camera)
    └── shader/                # GLSL vertex and fragment shaders
```

## Architecture

The engine enforces a clean separation between core systems and application logic through the `Core::Logic` interface:

```cpp
// core/Logic/Logic.hpp
class Logic {
public:
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
};
```

The game loop in `Core::Game` calls `onUpdate()` and `onRender()` each frame. The application provides its own implementation:

```cpp
class GameLogic : public Core::Logic {
    void onUpdate() override;
    void onRender() override;
};
```

### Camera System

`Core::Graphics::Camera` is a base class that computes view and projection matrices. `FreeCam` extends it with WASD + QE movement and mouse-controlled yaw/pitch:

| Key | Action |
|-----|--------|
| W / S | Move forward / backward |
| A / D | Strafe left / right |
| Q / E | Move down / up |
| Mouse | Look around |
| F | Toggle mouse capture |
| Escape | Close window |

### Input System

`Core::Input` tracks three key states per key:

```cpp
enum KeyState { KEY_RELEASED, KEY_PRESSED, KEY_HELD };

Core::Input::isKeyPressed(GLFW_KEY_W);  // true only on the frame it was pressed
Core::Input::isKeyHeld(GLFW_KEY_W);     // true while held down
Core::Input::getDeltaTime();            // frame delta for consistent movement speed
```

## Dependencies

- [GLFW](https://www.glfw.org/) — windowing and input
- [GLEW](https://glew.sourceforge.net/) — OpenGL extension loading
- [GLM](https://github.com/g-truc/glm) — math library (vectors, matrices)
- [CMake](https://cmake.org/) 3.x+

GLFW and GLEW are vendored as static libraries under `core/Graphics/lib/`.

## Running Without Building

Download the latest pre-built binary from the [Releases](https://github.com/matheusCsousa/FirstGameEngine/releases) page.

```bash
chmod +x fersa
./fersa
```

> Linux only. Make sure you run the binary from the directory where the `gameApp/shader/` folder is located.

---

## Building

```bash
git clone https://github.com/matheusCsousa/FirstGameEngine.git
cd FirstGameEngine
mkdir build && cd build
cmake ..
make
```

The executable is output to `build/gameApp/fersa`.

## Running

```bash
./build/gameApp/fersa
```

> Shaders are loaded from relative paths, so run from the project root.

## Creating a New Application

1. Create a class that inherits `Core::Logic`
2. Configure `Core::GameSpecs` and call `game.pushLogic<YourLogic>()`

```cpp
#include "core/Game/Game.hpp"
#include "YourLogic.hpp"

int main() {
    Core::GameSpecs specs;
    specs.title = "My Game";
    specs.windowSpec.width = 1280;
    specs.windowSpec.height = 720;

    Core::Game game(specs);
    game.pushLogic<YourLogic>();
    game.run();
}
```
