# Object scripting

The editor embeds Lua 5.4.9. Attach one `.lua` asset per object in Inspector > Script, create a template with New script, or import a source file. Edit script opens a source editor; Save (or Ctrl+S while this editor is open) checks syntax before replacing the existing file. Close offers Save/Discard/Cancel for unsaved source edits. Restart Play to reload saved code. Source-file edits are separate from scene undo; script attachment/detachment is part of scene history.

A script returns a table with optional `on_start(self)` and `on_update(self, dt)` functions:

```lua
return {
    on_start = function(self)
        self.speed = 45
        print("Starting", self.name)
    end,
    on_update = function(self, dt)
        self.rotation.y = self.rotation.y + self.speed * dt
    end
}
```

`on_start` runs once per Play session. `on_update` runs once per fixed 1/120-second simulation tick, before physics. `dt` is seconds and rotation values are degrees. Pause stops callbacks; Step invokes one tick while paused. Stop discards all script/runtime changes; a new Play session starts from authored data and fresh script state.

## self fields

- `self.position.x/y/z`: world position.
- `self.rotation.x/y/z`: XYZ Euler angles in degrees.
- `self.scale.x/y/z`: positive scale, 0.01–1000.
- `self.velocity.x/y/z`: physics velocity. Physics integrates it when mass is positive.
- `self.name`: refreshed from the owning entity; changing it does not rename the object.

The four vector fields are synchronized from the runtime object before each callback, then validated and committed together after it returns. Invalid values reject that callback's transform changes and disable that object's script. Numbers must be finite and within the supported range. Custom self fields such as `speed`, counters, or timers persist between callbacks. Each scripted object has separate globals and a separate Lua state, even when two objects share one source asset. Scripts do not access the authored scene.

To move a dynamic object, give it positive mass and set `self.velocity.x = 2` in `on_start`. To animate a static prop, change `self.position` or `self.rotation` directly in `on_update`.

## Errors and available libraries

Syntax errors are shown while saving/importing. Load/callback errors are reported with the object and script path in Assets > Script console. A failing script is disabled for that Play session; other scripts and physics continue. `print(...)` writes bounded messages to that console.

Exposed libraries are a subset of Lua base plus math, string, table, and utf8. File/process access, module loading, debug/coroutine libraries, dynamic source loading, metatable changes, protected-call helpers, and explicit garbage-collection controls are not exposed. There is no input, spawning, scene-query, or collision-callback API in this first version.

Limits: 256 KiB source files, 64 scripted objects per preview, 4 MiB Lua allocations per object, and about 200000 Lua instructions per chunk/callback. The instruction budget counts Lua VM instructions; it is not a wall-clock timeout for C library functions. Diagnostics are capped at 100 messages per session and callback error text is truncated. Saved script files reload on each Play; no live code reload runs during simulation.
