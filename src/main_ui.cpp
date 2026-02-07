// HyprLaunch UI - Standalone GTK4 process (NOT inside compositor!)
// Runs as a normal Wayland client. Receives commands via Unix socket.

#include "hyprlaunch/LauncherRenderer.hpp"
#include "hyprlaunch/AppDiscovery.hpp"
#include "hyprlaunch/ConfigParser.hpp"
#include <gtk/gtk.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace hyprlaunch;

static const char* SOCKET_PATH = "/tmp/hyprlaunch-ui.sock";
static LauncherRenderer* g_renderer = nullptr;
static int g_listenSock = -1;

// ============================================================================
// Send command to existing UI instance (returns true if sent)
// ============================================================================

static bool sendCommand(const char* cmd) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) return false;

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
        close(sock);
        return false;
    }

    ssize_t written = write(sock, cmd, strlen(cmd));
    close(sock);
    return written > 0;
}

// ============================================================================
// Socket listener (accepts commands from plugin via fork+exec)
// ============================================================================

static gboolean onSocketAccept(GIOChannel*, GIOCondition, gpointer) {
    struct sockaddr_un clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSock = accept(g_listenSock,
        reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
    if (clientSock == -1) return TRUE;

    char buf[64] = {};
    ssize_t n = read(clientSock, buf, sizeof(buf) - 1);
    close(clientSock);

    if (n > 0 && g_renderer) {
        std::string cmd(buf, static_cast<size_t>(n));
        if (cmd == "toggle") g_renderer->toggle();
        else if (cmd == "show") g_renderer->show();
        else if (cmd == "hide") g_renderer->hide();
        else if (cmd == "apps") {
            g_renderer->setMode(LauncherMode::Apps);
            if (!g_renderer->isVisible()) g_renderer->show();
        }
        else if (cmd == "helpers") {
            g_renderer->setMode(LauncherMode::Helpers);
            if (!g_renderer->isVisible()) g_renderer->show();
        }
    }

    return TRUE;
}

static bool createSocketListener() {
    unlink(SOCKET_PATH);

    g_listenSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_listenSock == -1) return false;

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(g_listenSock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
        close(g_listenSock);
        g_listenSock = -1;
        return false;
    }

    listen(g_listenSock, 5);

    GIOChannel* channel = g_io_channel_unix_new(g_listenSock);
    g_io_add_watch(channel, G_IO_IN, onSocketAccept, nullptr);
    g_io_channel_unref(channel);

    return true;
}

// ============================================================================
// Signal handler (clean shutdown)
// ============================================================================

static GMainLoop* g_mainLoop = nullptr;

static void onSignal(int) {
    if (g_mainLoop) g_main_loop_quit(g_mainLoop);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    // Parse command
    std::string cmd;
    LauncherMode startMode = LauncherMode::Apps;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--toggle" || arg == "toggle") cmd = "toggle";
        else if (arg == "--show" || arg == "show") cmd = "show";
        else if (arg == "--hide" || arg == "hide") cmd = "hide";
        else if (arg == "--apps" || arg == "apps") {
            cmd = "apps";
            startMode = LauncherMode::Apps;
        }
        else if (arg == "--helpers" || arg == "helpers") {
            cmd = "helpers";
            startMode = LauncherMode::Helpers;
        }
    }

    // If we have a command, try sending to existing instance first
    if (!cmd.empty()) {
        if (sendCommand(cmd.c_str())) {
            return 0;  // Sent to running instance, done
        }
        // No running instance -> start one and execute command
    }

    // Initialize GTK (safe: we're a separate Wayland client, NOT inside compositor)
    gtk_init();

    // Load config
    Config config = loadConfig();

    // Create components
    AppDiscovery discovery(config);
    LauncherRenderer renderer(config, discovery);
    g_renderer = &renderer;

    // Create UI (window, CSS, widgets - but don't show yet)
    renderer.initialize();

    // Set up socket listener for commands from plugin
    createSocketListener();

    // If started with a command, execute it now
    if (!cmd.empty()) {
        if (cmd == "helpers") {
            renderer.setMode(LauncherMode::Helpers);
        } else {
            renderer.setMode(startMode);
        }

        if (cmd == "toggle" || cmd == "show" || cmd == "apps" || cmd == "helpers") {
            renderer.show();
        }
    }

    // Signal handling
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    // Run GLib main loop
    g_mainLoop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(g_mainLoop);
    g_main_loop_unref(g_mainLoop);

    // Cleanup
    g_renderer = nullptr;
    unlink(SOCKET_PATH);
    if (g_listenSock >= 0) close(g_listenSock);

    return 0;
}
