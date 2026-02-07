# CLAUDE.md

## MANDATORY WORKFLOW

- **QUESTIONS ALWAYS HAVE PRIORITY** - If the user's request contains questions, answer ALL questions FIRST before implementing anything
- **NEVER implement while questions are unanswered** - This ensures the implementation matches user intent
- **NEVER ask questions you can answer yourself** - Research first using web search, file reads, or other tools before asking the user
- **Only ask questions that require user decision** - Technical facts, versions, documentation can be researched independently
- **NEVER invent values when you have correct data** - If you researched a value (version, number, name), use EXACTLY that value, do not make up different values

## STRICTLY FORBIDDEN

- **NEVER use `sudo`** - no exceptions, no matter what
- **NO workarounds** - always implement the correct solution
- **NO hacks** - clean, correct code only
- **NO shortcuts** - complete implementations required
- **NO tricks** - use standard solutions
- **NEVER delete code to avoid implementation** - implement the code, don't remove it
- **NO stubs** - all functions must be fully implemented
- **NO TODOs** - do it right immediately
- **NO orphaned code** - all functions/methods must be used; if you implement something, use it
- **NO dead code** - remove unused code, don't leave it hanging

## FORBIDDEN COMMANDS

- **NEVER copy files to `/var/cache/hyprpm/`** - this is a system cache managed by hyprpm
- **NEVER use `hyprctl plugin load`** - the user manages plugin loading themselves
- **NEVER use `hyprpm update`** - this pulls from GitHub, not local changes
- **NEVER write to system directories** - stay within the project directory

## PROJECT: HyprLaunch

Layer-shell app launcher for Hyprland. 1:1 port of the AGS app launcher (TypeScript) to C++.

### Architecture (TWO components)

1. **hyprlaunch.so** - Hyprland plugin (loaded into compositor)
   - NO GTK, NO threads, NO blocking calls
   - Dispatchers: `hyprlaunch:toggle`, `hyprlaunch:show`, `hyprlaunch:hide`, `hyprlaunch:apps`, `hyprlaunch:helpers`
   - IPC via `hyprctl hyprlaunch:<command>` (e.g. `hyprctl hyprlaunch:helpers`)
   - Communicates with UI via `fork()` + `execlp("hyprlaunch-ui")`

2. **hyprlaunch-ui** - Standalone GTK4 binary (separate Wayland client process)
   - GTK4 + gtk4-layer-shell for overlay window
   - Receives commands via Unix socket `/tmp/hyprlaunch-ui.sock`
   - App discovery via GDesktopAppInfo
   - Helper scripts from `~/.local/bin/helpers/*.sh`
   - Fuzzy search, recent apps, calculator mode
   - Keyboard navigation: Up/Down/PgUp/PgDn/Home/End/Tab/Enter/Escape

### Critical Safety Rules

- **NEVER call `gtk_init()` inside the compositor plugin** - this WILL deadlock and freeze the entire system
- **NEVER use threads in the plugin** - Hyprland is single-threaded
- **NEVER use `popen()`/`system()` in the plugin main thread** - use `fork()` + `execlp()` instead
- **NEVER block the compositor** - all slow operations must happen in forked child processes

### Build

```bash
./build.sh          # Release build (both targets)
./build.sh debug    # Debug build
./build.sh clean    # Clean build directory
./build.sh install  # Build + install plugin + UI binary
```

### Tech Stack

- C++23, CMake, PkgConfig
- Hyprland 0.53.1+ plugin API
- GTK4 + gtk4-layer-shell (UI binary only)
- GDesktopAppInfo (app discovery)

### Key Files

- `src/main.cpp` - Plugin entry point (dispatchers, IPC, lifecycle)
- `src/Globals.cpp` - Plugin globals, fork+exec UI
- `src/IPCHandler.cpp` - hyprctl command handling
- `src/main_ui.cpp` - UI binary entry point (socket listener, GTK main loop)
- `src/LauncherRenderer.cpp` - GTK4 layer-shell window, all UI logic
- `src/AppDiscovery.cpp` - App loading, search, launch logic
- `src/ConfigParser.cpp` - Config file reader

### Reference

- Original AGS template: `~/.config/iconmanager/templates/ags-launcher-zofi.template`
- HyprClipX reference: `/mnt/code/SRC/GITHUB/hyprclipx/`
- Correct plugin patterns: `/mnt/code/SRC/GITHUB/hyprzones/`
