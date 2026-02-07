# Restarting HyprLaunch-UI

## Correct Method

```bash
# Stop old instance
killall -9 hyprlaunch-ui

# Press SUPER+SPACE or SUPER+SHIFT+H — plugin spawns a new instance
```

## One-Liner (kill + reopen)

```bash
killall -9 hyprlaunch-ui; sleep 0.3; hyprctl dispatch hyprlaunch:toggle
```

## Testing Dev Build (without hyprpm install)

```bash
killall -9 hyprlaunch-ui; sleep 0.3; /mnt/code/SRC/GITHUB/hyprlaunch/build/hyprlaunch-ui --show
```

## How It Works

- The **plugin** (`hyprlaunch.so`) runs inside the Hyprland compositor
- On each command it calls `fork()` + `execlp("hyprlaunch-ui", "--<cmd>")`
- **hyprlaunch-ui** is a standalone GTK4 layer-shell window

## Check if HyprLaunch-UI Is Running

```bash
pgrep -f hyprlaunch-ui
```

## When to Restart HyprLaunch-UI?

- After changes to UI code (LauncherRenderer.cpp, AppDiscovery.cpp, main_ui.cpp)
- After changes to CSS styles
- After changes to `~/.config/hypr/hyprlaunch.toml` (window size, etc.)
- After building with new features

## When to Reload the Plugin?

The plugin (`hyprlaunch.so`) only needs reload after changes to `src/main.cpp`:

```bash
hyprpm reload
```
