// HyprLaunch - Layer-shell app launcher for Hyprland
// Plugin entry point - LIGHTWEIGHT (no GTK, no threads)
// UI runs as separate process (hyprlaunch-ui)

#define WLR_USE_UNSTABLE
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

#include "hyprlaunch/Globals.hpp"
#include "hyprlaunch/IPCHandler.hpp"

using namespace hyprlaunch;

inline HANDLE g_pHandle = nullptr;

// ============================================================================
// IPC Command Handler (hyprctl hyprlaunch <cmd> [args])
// ============================================================================

static std::string cmdHyprlaunch(eHyprCtlOutputFormat, std::string request) {
    std::string cmd = request;
    std::string args;

    size_t spacePos = cmd.find(' ');
    if (spacePos != std::string::npos) {
        args = cmd.substr(spacePos + 1);
        cmd = cmd.substr(0, spacePos);
    }

    if (g_ipcHandler) {
        return g_ipcHandler->handleCommand(cmd, args);
    }
    return "error: not initialized";
}

// ============================================================================
// Dispatchers
// ============================================================================

static SDispatchResult dispatchShow(std::string) {
    sendUICommand("show");
    return {.success = true};
}

static SDispatchResult dispatchHide(std::string) {
    sendUICommand("hide");
    return {.success = true};
}

static SDispatchResult dispatchToggle(std::string) {
    sendUICommand("toggle");
    return {.success = true};
}

// ============================================================================
// Plugin Lifecycle
// ============================================================================

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    g_pHandle = handle;
    g_handle = handle;

    initGlobals();

    // Register IPC command
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch", true, cmdHyprlaunch});

    // Register dispatchers
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:show", dispatchShow);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:hide", dispatchHide);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:toggle", dispatchToggle);

    // Register config values
    HyprlandAPI::addConfigValue(handle, "plugin:hyprlaunch:enabled",
                                Hyprlang::INT{1});
    HyprlandAPI::addConfigValue(handle, "plugin:hyprlaunch:hotkey",
                                Hyprlang::STRING{"SUPER D"});

    HyprlandAPI::addNotification(handle,
        "[HyprLaunch] Loaded successfully!",
        CHyprColor(0.2f, 0.8f, 0.2f, 1.0f),
        5000);

    return {
        "hyprlaunch",
        "Layer-shell app launcher for Hyprland",
        "HyprLaunch",
        "0.1.0"
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    cleanupGlobals();

    HyprlandAPI::addNotification(g_pHandle,
        "[HyprLaunch] Unloaded",
        CHyprColor(0.8f, 0.8f, 0.2f, 1.0f),
        3000);
}
