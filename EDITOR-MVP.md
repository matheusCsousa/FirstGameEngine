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

Separate editor target, docking layout, scene hierarchy, stable entity selection, and transform inspector. Includes the framebuffer viewport and create/duplicate/delete actions to make transform editing immediately useful. Scene simulation stays paused in Edit mode.

### PR 3: scene persistence

Versioned scene documents, asset references, validated save/load, and dirty-state handling. The framebuffer viewport and camera input ownership are implemented with the editor shell.

### PR 4: authoring workflow

Play/Stop state isolation, transform gizmos, undo/redo, and physics/material inspection. Create/duplicate/delete are implemented with the editor shell.

## Verification

- Runtime unit tests for IDs/removal, scene-document round trips, and collision math.
- Smoke tests for startup, frame rendering, clean shutdown, and missing/invalid assets.
- Editor checks for selection after deletion, invalid scene-file recovery, viewport resizing/minimization, camera input while typing, and Edit/Play/Stop state isolation.

The runtime foundation is implemented. The editor-shell changes add named entities with stable scene-local IDs, safe removal/duplication, a separate docking editor, a live framebuffer viewport, and a transform inspector. Physics remains paused while authoring. Play/Pause/Step/Stop now uses a disposable scene copy and a bounded fixed-step physics preview, with read-only runtime inspection. Scene persistence is now implemented with versioned, validated files and dirty-state prompts; see SCENE-FORMAT.md. Viewport selection, gizmos, and undo/redo are now implemented. The remaining authoring workflow includes materials/assets and per-object scripting.

## Per-object scripting (later milestone)

Users should be able to attach scripts to scene objects and program their behavior. Plan a script component containing a portable script asset reference and editable properties, a runtime lifecycle (start/update/stop), access to the owning entity's transform, and useful error reporting. Scripts must run in the Play-mode scene copy so stopping restores authored state. Choose the scripting language and reload behavior in that milestone; no script execution or attachment UI is implemented yet. Extend the versioned scene format when script components are introduced.

## Implemented Play workflow

Play clones mutable scene/entity data and the editor camera while sharing GPU assets. The authored document, its selection IDs, save path, and dirty state remain intact. Pause and single-step operate on the runtime copy; Stop destroys it. Editing and file operations are disabled until Stop, and native close stops preview before checking the authored dirty state. Physics inspection exposes mass, gravity, and initial velocity in Edit mode; mass zero is static. The preview currently simulates physics only and does not execute scripts.

Fixed stepping uses 1/120 second, a maximum of eight ticks per frame, and no catch-up for paused time. Regression tests cover state isolation, restored transforms, replay, pause/resume/step, bounded/invalid frame times, equal-depth collision axes, and falling-body landing.

## Implemented viewport authoring

Click selection casts a ray through the viewport into each object's local bounds using its inverse model transform, preserving distance for nearest-hit selection. This fits the current cube-only scene format and does not claim arbitrary triangle picking. Selected bounds are outlined. ImGuizmo provides move/rotate/scale handles; XYZ matrix decomposition matches the core model transform order. Scaling stays local and positive; reflected/sheared/non-finite transformations are rejected.

Scene history stores document snapshots plus selection for restoration, groups continuous gestures, resets on New/Open, preserves save checkpoints, and discards redo branches after a new edit. It restores shared asset references through the cube preset and preserves the entity ID allocator's high-water mark across undo. Camera movement and selection alone are not recorded. Play blocks viewport editing and history operations. History is not serialized.

Native checks cover viewport selection, all three gizmo operations, a complete drag undone/redone with the dirty marker restored, and disabled tools in Play. Automated checks cover rotated/nonuniform bounds, nearest-hit/behind-ray misses, viewport/DPI mapping, XYZ matrix round trips and invalid matrices, history branches, checkpoints, and snapshot/memory limits.
