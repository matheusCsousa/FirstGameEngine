# Scene files, version 2

UTF-8 JSON contains `format: "fersa-scene"`, integer `version: 2`, and an `objects` array. Empty scenes are valid. Version 1 cube/brick scenes remain readable and receive default tint, UV tiling, and no script when loaded. Saves write version 2.

Each object contains unique, nonzero unsigned 64-bit `id` (maximum uint64 reserved), `name`, `mesh`, `material`, `script`, `tint` (three numbers), `tiling` (two numbers), three-number `position`, `rotation` (degrees), and `scale`, plus `physics` containing `mass`, `gravity`, `velocity`, and `acceleration`.

Mesh is `models/cube.obj` for the built-in cube or a project-relative `assets/...obj` reference. Material is `builtin/brick`, `builtin/white`, or a project-relative PNG/JPG/JPEG reference. Script is empty or a project-relative Lua reference. Tint is 0–1 per component; tiling is 0.01–100. There are no serialized GPU handles, pointers, editor-camera state, dock layout, script runtime variables, or undo history.

The scene's parent directory is the project root. Import copies validated source files into its `assets/` folder, sanitizing names and avoiding overwrite with numeric suffixes. Relative references cannot contain absolute paths, traversal, backslashes, NULs, or drive separators. Resolution checks canonical containment and rejects links escaping the project. General OBJ assets use the engine surface shader and one per-object texture/tint; MTL material sets are not imported.

Saving to another folder copies managed project assets, including unassigned files needed by undo history, before writing the scene. Existing identical asset files are reused; differing files cause a conflict and remain untouched. A failed operation may leave newly copied asset files in the destination, but does not overwrite conflicting files or the prior scene. To move a project manually, keep its scene and assets folder together. Built-ins still come from the editor's configured asset root.

The whole document and all referenced resources are loaded into a replacement scene before replacing the active scene. Missing/invalid models or textures preserve the active scene. Script source must exist and fit the file limit; syntax/runtime failures can be diagnosed in the script editor or Play console. Unknown scene versions and invalid field types/IDs/numbers are rejected.

Transforms and physics numbers are finite, scale is 0.01–1000, mass is nonnegative, names are limited to 1024 UTF-8 bytes, references to 512 bytes, and files to 4 MiB / 10000 objects / 32 nesting levels. OBJ files are limited to 64 MiB, one million emitted vertices, and supported finite coordinates. Images are limited to 64 MiB input, 8192 per axis, 16 megapixels, and the current GPU texture limit. Lua source is limited to 256 KiB.

Scene saving validates and writes a sibling temporary file, checks stream errors, and replaces the destination (MoveFileExW on Windows, rename on POSIX). Failed saves keep the document dirty. New/Open/native close use Save/Discard/Cancel for unsaved authored data. Script source has its own save prompt. New keeps the current project asset library; Open changes project root to the chosen scene directory. Files are local; no cloud synchronization is implemented.

Built-in mesh references also include `models/square.obj`, `models/plane.obj`,
`models/sphere.obj`, `models/cylinder.obj`, `models/cone.obj`, and
`models/capsule.obj`. These ship with the editor and need no project import.
