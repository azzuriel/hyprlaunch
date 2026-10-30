// HyprLaunch - Layer-shell app launcher for Hyprland
// Plugin entry point - LIGHTWEIGHT (no GTK, no threads)
// UI runs as separate process (hyprlaunch-ui)

#define WLR_USE_UNSTABLE
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/ipc/s1/S1.hpp>

#include <lua.h>

#include "hyprlaunch/Globals.hpp"

using namespace hyprlaunch;

inline HANDLE g_pHandle = nullptr;

// ============================================================================
// IPC Command Handlers (hyprctl hyprlaunch:<cmd>)
// ============================================================================

static IPC::Socket1::SResponse cmdShow(const IPC::Socket1::SRequest&) {
    sendUICommand("show");
    return "ok";
}

static IPC::Socket1::SResponse cmdHide(const IPC::Socket1::SRequest&) {
    sendUICommand("hide");
    return "ok";
}

static IPC::Socket1::SResponse cmdToggle(const IPC::Socket1::SRequest&) {
    sendUICommand("toggle");
    return "ok";
}

static IPC::Socket1::SResponse cmdApps(const IPC::Socket1::SRequest&) {
    sendUICommand("apps");
    return "ok";
}

static IPC::Socket1::SResponse cmdHelpers(const IPC::Socket1::SRequest&) {
    sendUICommand("helpers");
    return "ok";
}

static IPC::Socket1::SResponse cmdReload(const IPC::Socket1::SRequest&) {
    reloadConfig();
    return "config reloaded";
}

// ============================================================================
// Lua functions (hl.plugin.hyprlaunch.<fn>) - Lua config mode
// ============================================================================

static int luaShow(lua_State*) {
    sendUICommand("show");
    return 0;
}

static int luaHide(lua_State*) {
    sendUICommand("hide");
    return 0;
}

static int luaToggle(lua_State*) {
    sendUICommand("toggle");
    return 0;
}

static int luaApps(lua_State*) {
    sendUICommand("apps");
    return 0;
}

static int luaHelpers(lua_State*) {
    sendUICommand("helpers");
    return 0;
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
        IPC::Socket1::SCommand{.name = "hyprlaunch:show", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdShow});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        IPC::Socket1::SCommand{.name = "hyprlaunch:hide", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdHide});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        IPC::Socket1::SCommand{.name = "hyprlaunch:toggle", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdToggle});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        IPC::Socket1::SCommand{.name = "hyprlaunch:apps", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdApps});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        IPC::Socket1::SCommand{.name = "hyprlaunch:helpers", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdHelpers});
    HyprlandAPI::registerHyprCtlCommand(g_handle,
        IPC::Socket1::SCommand{.name = "hyprlaunch:reload", .match = IPC::Socket1::COMMAND_MATCH_EXACT, .handler = cmdReload});

    // Register Lua functions (hl.plugin.hyprlaunch.<fn>)
    HyprlandAPI::addLuaFunction(handle, "hyprlaunch", "show", luaShow);
    HyprlandAPI::addLuaFunction(handle, "hyprlaunch", "hide", luaHide);
    HyprlandAPI::addLuaFunction(handle, "hyprlaunch", "toggle", luaToggle);
    HyprlandAPI::addLuaFunction(handle, "hyprlaunch", "apps", luaApps);
    HyprlandAPI::addLuaFunction(handle, "hyprlaunch", "helpers", luaHelpers);

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
