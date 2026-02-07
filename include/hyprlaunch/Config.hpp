#pragma once
#include <string>

namespace hyprlaunch {

struct Config {
    int windowWidth = 550;
    int windowHeight = 550;

    std::string hotkey = "SUPER D";

    // Paths (set at runtime from $HOME)
    std::string recentFile;   // ~/.cache/hyprlaunch-recent.json
    std::string helpersDir;   // ~/.local/bin/helpers
};

} // namespace hyprlaunch
