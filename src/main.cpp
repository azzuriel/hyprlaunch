// HyprLaunch - Layer-shell app launcher for Hyprland
// Plugin entry point - LIGHTWEIGHT (no GTK, no threads)
// UI runs as separate process (hyprlaunch-ui)

#define WLR_USE_UNSTABLE
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

#include "hyprlaunch/Globals.hpp"

using namespace hyprlaunch;

inline HANDLE g_pHandle = nullptr;

// ============================================================================
// IPC Command Handlers (hyprctl hyprlaunch:<cmd>)
// ============================================================================

static std::string cmdShow(eHyprCtlOutputFormat, std::string) {
    sendUICommand("show");
    return "ok";
}

static std::string cmdHide(eHyprCtlOutputFormat, std::string) {
    sendUICommand("hide");
    return "ok";
}

static std::string cmdToggle(eHyprCtlOutputFormat, std::string) {
    sendUICommand("toggle");
    return "ok";
}

static std::string cmdApps(eHyprCtlOutputFormat, std::string) {
    sendUICommand("apps");
    return "ok";
}

static std::string cmdHelpers(eHyprCtlOutputFormat, std::string) {
    sendUICommand("helpers");
    return "ok";
}

static std::string cmdReload(eHyprCtlOutputFormat, std::string) {
    reloadConfig();
    return "config reloaded";
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

static SDispatchResult dispatchApps(std::string) {
    sendUICommand("apps");
    return {.success = true};
}

static SDispatchResult dispatchHelpers(std::string) {
    sendUICommand("helpers");
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

    // Register IPC commands (hyprctl hyprlaunch:<cmd>)
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:show", true, cmdShow});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:hide", true, cmdHide});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:toggle", true, cmdToggle});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:apps", true, cmdApps});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:helpers", true, cmdHelpers});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        SHyprCtlCommand{"hyprlaunch:reload", true, cmdReload});

    // Register dispatchers
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:show", dispatchShow);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:hide", dispatchHide);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:toggle", dispatchToggle);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:apps", dispatchApps);
    HyprlandAPI::addDispatcherV2(handle, "hyprlaunch:helpers", dispatchHelpers);

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
