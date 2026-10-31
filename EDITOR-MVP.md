# Visual scene editor: first milestone

## Direction

The intended product is an editor with visual scene-building tools, backed by the existing C++/OpenGL runtime.

## First usable editor

1. **Scene hierarchy:** named entities with stable IDs; select, create a cube, duplicate, and delete.
2. **Inspector:** edit position, rotation, scale, and basic physics properties for the selected entity. Validate numeric values and prevent stale selection after deletion.
3. **Viewport:** render the scene with an editor camera; resize the framebuffer safely; camera controls operate only when the viewport owns input.
4. **Scene files:** versioned JSON containing entity IDs, names, transforms, physics properties, and asset references. GPU handles and raw pointers are never serialized.
5. **Edit/Play/Stop:** Edit mode does not simulate physics. Play operates on a runtime copy; Stop restores the edited scene rather than saving simulated transforms accidentally.
6. **Dirty state:** indicate unsaved changes and offer Save/Discard/Cancel before replacing a scene or closing the editor.

A first working version should let the user create two objects, move and scale them visually through inspector controls, save, reopen, and run the scene without losing the authored state. Transform gizmos, undo/redo, and drag-and-drop assets follow this milestone.

## Recommended UI integration

Use Dear ImGui with its GLFW/OpenGL backends and docking support. Keep editor UI code in a separate `editor/` application/target. The core engine remains usable by the existing `gameApp/` target without depending on editor widgets.

## Required runtime foundation

- Fix `core/Scene/Scene.hpp`: its camera include points to `../Camera/Camera.hpp`, while the actual camera header lives under `../Graphics/Camera/Camera.hpp`.
- Establish a clean, documented dependency build, including explicit GLM package/target handling.
- Ensure startup failures stop execution and scenes/GPU resources are destroyed before the OpenGL context.
- Make Camera's destructor virtual before continuing polymorphic ownership (`Scene` owns `unique_ptr<Camera>`, while the sample constructs `FreeCam`).
- Introduce entity IDs/names and scene-owned removal; avoid retaining editor selections as raw pointers.
- Separate camera update, simulation update, and rendering. `Scene::update()` currently always runs camera movement and physics together.
- Supply delta time from the game loop and support a fixed physics step.
- Introduce reusable asset references and a runtime asset root independent of the process working directory.

## Implementation order

### PR 1: runtime foundation

Build configuration, corrected includes, initialization/cleanup lifecycle, virtual camera destruction, and an automated sample smoke run. Establish a passing baseline before connecting GUI callbacks.

### PR 2: editor shell and selection

Separate editor target, docking layout, scene hierarchy, stable entity selection, and transform inspector. Scene simulation stays paused in Edit mode.

### PR 3: viewport and scene persistence

Framebuffer-backed viewport, correct viewport resize/input ownership, versioned scene documents, asset references, and validated save/load.

### PR 4: authoring workflow

Create/duplicate/delete, Play/Stop state isolation, dirty-state handling, transform gizmos, and undo/redo.

## Verification

- Runtime unit tests for IDs/removal, scene-document round trips, and collision math.
- Smoke tests for startup, frame rendering, clean shutdown, and missing/invalid assets.
- Editor checks for selection after deletion, invalid scene-file recovery, viewport resizing/minimization, camera input while typing, and Edit/Play/Stop state isolation.

The runtime-foundation changes implement the first milestone: dependency setup, lifecycle/failure handling, camera ownership, asset paths, and runtime checks. The local Windows build and all three local tests passed. The editor executable and its panels remain the next milestone.
