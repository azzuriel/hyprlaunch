# HyprLaunch

[![License](https://img.shields.io/badge/license-BSD--3--Clause-blue.svg)](LICENSE) &nbsp; [![Hyprland](https://img.shields.io/badge/Hyprland-0.53%2B-blue.svg)](https://hyprland.org) &nbsp; [![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23) &nbsp; [![Build](https://img.shields.io/badge/build-CMake-green.svg)](CMakeLists.txt)

A Hyprland plugin for a **layer-shell app launcher** with fuzzy search, recent apps, calculator mode and helper scripts.

## Overview

HyprLaunch is a 1:1 port of the AGS app launcher to native C++. It runs as a GTK4 layer-shell overlay centered on the focused monitor, with app discovery via GDesktopAppInfo, fuzzy search scoring, and a dual-mode launcher for desktop apps and helper scripts.

```
┌─────────────────────────────────────────────────────────────────┐
│ Hyprland Compositor                                             │
│   hyprlaunch.so plugin (no GTK, no threads, no blocking)        │
│     → dispatchers, IPC, fork+exec                               │
└──────────────────────┬──────────────────────────────────────────┘
                       │ fork() + execlp()
                       ▼
┌─────────────────────────────────────────────────────────────────┐
│ hyprlaunch-ui (standalone Wayland client)                        │
│   GTK4 + gtk4-layer-shell overlay window                        │
│     → fuzzy search, recent apps, calculator, keyboard nav       │
└──────────────────────┬──────────────────────────────────────────┘
                       │ GDesktopAppInfo / filesystem
                       ▼
┌─────────────────────────────────────────────────────────────────┐
│ Desktop Apps (.desktop files) + Helper Scripts (~/.local/bin/)  │
│   → app discovery, icon lookup, launch via GIO                  │
└─────────────────────────────────────────────────────────────────┘
```

## Features

### App Discovery
- **Desktop apps** - Discovers all installed applications via GDesktopAppInfo
- **Helper scripts** - Lists scripts from `~/.local/bin/helpers/*.sh` with descriptions
- **Dual mode** - Switch between apps and helpers via IPC command
- **App icons** - Native GTK4 icon theme lookup with fallback

### Search
- **Fuzzy search** - Substring matching with weighted scoring (name × 10 > description > keywords)
- **Recent apps** - Last 10 launched apps shown first when search is empty
- **Calculator mode** - Prefix `=` for math expressions (e.g. `=2+2`), Enter copies result
- **Real-time filtering** - Results update as you type

### Keyboard Navigation
- **Arrow keys** - Navigate up/down through results
- **Enter** - Launch selected app or copy calculator result
- **Tab** - Wrap-around to next result
- **Page Up/Down** - Jump 5 entries at a time
- **Home/End** - Jump to first/last result
- **Escape** - Close launcher

### Layer-Shell Overlay
- **Dark theme** - ASE Muted Dark Theme matching HyprClipX / HyprZones palette
- **Centered** - Automatically centered on the focused monitor
- **Auto-hide** - Closes after launching an app or on Escape
- **Singleton** - Only one instance runs, subsequent calls toggle via Unix socket

## Installation

### Using hyprpm (Recommended)

```bash
hyprpm add https://github.com/azzuriel/hyprlaunch
hyprpm enable hyprlaunch
hyprpm reload
```

### Manual Build

#### Requirements

| Package | Arch Linux | Description |
|---------|------------|-------------|
| Hyprland 0.53+ | `hyprland` | Wayland compositor with headers |
| CMake 3.19+ | `cmake` | Build system |
| GCC 13+ / Clang 17+ | `gcc` | C++23 compiler |
| pkg-config | `pkgconf` | Dependency resolver |
| GTK4 | `gtk4` | UI toolkit (for hyprlaunch-ui) |
| Gtk4LayerShell | `gtk4-layer-shell` | Wayland layer shell for GTK4 |
| GIO | `glib2` | App discovery (GDesktopAppInfo) |

#### Runtime Dependencies

| Package | Arch Linux | Description |
|---------|------------|-------------|
| wl-clipboard | `wl-clipboard` | Clipboard access for calculator (`wl-copy`) |
| bc | `bc` | Calculator expression evaluation |

#### Arch Linux

```bash
# Build dependencies
sudo pacman -S hyprland cmake gcc pkgconf gtk4 gtk4-layer-shell glib2

# Runtime dependencies
sudo pacman -S wl-clipboard bc
```

#### Build

```bash
git clone https://github.com/azzuriel/hyprlaunch
cd hyprlaunch
./build.sh
```

#### Install

```bash
./build.sh install
# Installs hyprlaunch.so to /usr/lib/hyprland/plugins/
# Installs hyprlaunch-ui to /usr/local/bin/
```

Add to `~/.config/hypr/hyprland.conf`:

```ini
plugin = /usr/lib/hyprland/plugins/hyprlaunch.so
```

## Configuration

Add to `~/.config/hypr/hyprland.conf`:

```ini
# Plugin settings
plugin:hyprlaunch:enabled = 1
plugin:hyprlaunch:hotkey = SUPER D

# Keybinding
bind = $mainMod, D, hyprlaunch:toggle
```

Optional config file at `~/.config/hypr/hyprlaunch.toml`:

```toml
[window]
window_width = 550
window_height = 550

[general]
hotkey = "SUPER D"
```

## Usage

### Keybindings

Add to `~/.config/hypr/hyprland.conf`:

```ini
# Toggle app launcher
bind = $mainMod, D, hyprlaunch:toggle

# Show / Hide
bind = $mainMod SHIFT, D, hyprlaunch:show
bind = , Escape, hyprlaunch:hide
```

### IPC Commands

```bash
# Toggle launcher window
hyprctl hyprlaunch:toggle

# Show / Hide
hyprctl hyprlaunch:show
hyprctl hyprlaunch:hide

# Mode switching
hyprctl hyprlaunch:apps       # Show in apps mode
hyprctl hyprlaunch:helpers    # Show in helpers mode

# Reload config
hyprctl hyprlaunch:reload

# Via dispatcher (same effect)
hyprctl dispatch hyprlaunch:toggle
hyprctl dispatch hyprlaunch:show
hyprctl dispatch hyprlaunch:helpers
```

### Keyboard Controls (Inside Launcher Window)

| Key | Action |
|-----|--------|
| Up / Down | Navigate results |
| Enter | Launch selected app / copy calculator result |
| Tab | Next result (wrap-around) |
| Page Up / Page Down | Jump 5 entries |
| Home / End | First / last result |
| Escape | Close launcher |

### Calculator Mode

Type `=` followed by a math expression:

| Input | Result |
|-------|--------|
| `=2+2` | 4 |
| `=100/3` | 33.333333 |
| `=2^10` | 1024 |
| `=(5+3)*2` | 16 |

Press Enter to copy the result to clipboard via `wl-copy`.

### Helper Scripts

Helper scripts are `.sh` files in `~/.local/bin/helpers/`. The first comment line after the shebang is used as the description.

```bash
#!/bin/bash
# Update all system packages
pacman -Syu
```

Scripts containing `read`, `dialog`, `whiptail`, or `select` are automatically launched in a kitty terminal.

## Project Structure

```
hyprlaunch/
├── include/hyprlaunch/          # Header files
│   ├── Config.hpp               # Plugin configuration
│   ├── AppEntry.hpp             # App entry data structure
│   ├── AppDiscovery.hpp         # App loading, search, launch
│   ├── LauncherRenderer.hpp     # GTK4 layer-shell UI
│   ├── ConfigParser.hpp         # Config file reader
│   ├── IPCHandler.hpp           # hyprctl command handling
│   ├── Globals.hpp              # Plugin globals
│   └── Forward.hpp              # Forward declarations
├── src/
│   ├── main.cpp                 # Plugin entry (dispatchers, IPC, lifecycle)
│   ├── Globals.cpp              # Fork+exec UI
│   ├── IPCHandler.cpp           # hyprctl command routing
│   ├── ConfigParser.cpp         # Config value parsing
│   ├── main_ui.cpp              # UI binary entry (socket listener, GTK loop)
│   ├── LauncherRenderer.cpp     # GTK4 window, CSS, widgets, keyboard nav
│   └── AppDiscovery.cpp         # App discovery, search, calculator, launch
├── docs/
│   ├── ARCH_HYPRLAUNCH_LAUNCHER.md  # Launcher architecture
│   └── WRFL_HYPRLAUNCH_RESTART.md   # Restart workflow
├── build.sh                     # Build script
├── CMakeLists.txt               # Two targets: .so plugin + UI binary
├── CLAUDE.md                    # AI assistant guidelines
└── README.md
```

## Troubleshooting

### Launcher doesn't appear

1. Check if plugin is loaded: `hyprctl plugins list`
2. Check if UI binary is in PATH: `which hyprlaunch-ui`
3. Try manual toggle: `hyprctl dispatch hyprlaunch:toggle`
4. Try UI standalone: `hyprlaunch-ui --show`

### No apps listed

1. Check if desktop files exist: `ls /usr/share/applications/`
2. Verify GIO can find apps: `gio list applications`
3. Check for errors: `hyprlaunch-ui --show 2>&1`

### Calculator not working

1. Check if `bc` is installed: `which bc`
2. Install: `sudo pacman -S bc`

### Helper scripts not showing

1. Check directory exists: `ls ~/.local/bin/helpers/`
2. Verify scripts are executable: `chmod +x ~/.local/bin/helpers/*.sh`
3. Files must end in `.sh` and not contain `.backup.` in the name

### Plugin freezes compositor

This should not happen with the current architecture (plugin has no GTK, no threads). If it does:
1. Switch to TTY: Ctrl+Alt+F2
2. Remove plugin: `sed -i '/hyprlaunch/d' ~/.config/hypr/hyprland.conf`
3. Restart Hyprland

## License

BSD 3-Clause License (same as Hyprland)

## Credits

- 1:1 port of the [AGS app launcher](https://github.com/Aylur/ags)
- Built for [Hyprland](https://hyprland.org)
- UI theme matching [HyprClipX](https://github.com/azzuriel/hyprclipx) and [HyprZones](https://github.com/azzuriel/hyprzones)

## Contributors

- [Leon Marzoll](https://github.com/LeonMarzollDev) — Key contributor to the original version of the app launcher that this project is based on
