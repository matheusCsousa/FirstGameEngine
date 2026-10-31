# Scene files, version 1

The editor writes UTF-8 JSON with `format: "fersa-scene"`, integer `version: 1`, and an `objects` array. Empty scenes are valid.

Each object contains a nonzero unsigned 64-bit `id`, `name`, `mesh`, `material`, three-number `position`, `rotation` (degrees), and `scale` arrays, and a `physics` object containing `mass`, `gravity`, `velocity`, and `acceleration`. IDs are unique within the scene; the maximum uint64 value is reserved. Restoring IDs advances the scene's allocator so later objects do not reuse them.

Only `mesh: "models/cube.obj"` and `material: "builtin/brick"` are supported in this version. Material resolves to the bundled shader and brick texture, relative to the configured asset root. Arbitrary asset paths and script references are not supported. Scene files do not contain GPU handles, raw pointers, editor camera state, or dock layouts.

Numbers must be finite, scale must be 0.01–1000 on every axis, mass must be nonnegative, names are limited to 1024 UTF-8 bytes, and files are limited to 4 MiB / 10000 objects / 32 JSON nesting levels. Unknown fields are ignored; unknown versions, malformed fields, duplicate IDs, and unsupported assets are rejected. The whole document is validated and a replacement scene is built before the current scene is changed.

Saving validates and writes a sibling temporary file, checks stream errors, and then replaces the destination (MoveFileExW on Windows, rename on POSIX). A failed validation or replacement preserves the previous destination and leaves the editor dirty. This does not promise recovery from sudden hardware power loss. Files are local; no cloud synchronization is implemented.

File > Save/Open uses an explicit path dialog. New/Open/Exit/native close defer to Save/Discard/Cancel when dirty. Cancelling a path dialog or failing to read/write keeps the current scene. New opens an empty, clean untitled scene. The initial sample is clean until the user edits it.
