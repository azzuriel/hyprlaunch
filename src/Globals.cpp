// Global instance management (plugin side - NO GTK!)
// Uses fork+exec to launch UI binary

#include "hyprlaunch/Globals.hpp"
#include "hyprlaunch/IPCHandler.hpp"
#include "hyprlaunch/ConfigParser.hpp"

#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

namespace hyprlaunch {

std::unique_ptr<IPCHandler> g_ipcHandler;
Config g_config;
void* g_handle = nullptr;

void initGlobals() {
    reloadConfig();
    g_ipcHandler = std::make_unique<IPCHandler>();
}

void cleanupGlobals() {
    g_ipcHandler.reset();
}

void reloadConfig() {
    g_config = loadConfig();
}

void sendUICommand(const std::string& cmd) {
    // Reap zombie children from previous calls
    while (waitpid(-1, nullptr, WNOHANG) > 0) {}

    if (fork() == 0) {
        setsid();  // Detach from compositor process group
        std::string arg = "--" + cmd;
        execlp("hyprlaunch-ui", "hyprlaunch-ui", arg.c_str(), nullptr);
        _exit(1);
    }
}

} // namespace hyprlaunch
