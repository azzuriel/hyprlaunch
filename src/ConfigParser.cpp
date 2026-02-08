// Configuration file parsing

#include "hyprlaunch/ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

namespace hyprlaunch {

std::string getConfigPath() {
    const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
    std::string configDir;

    if (xdgConfig) {
        configDir = xdgConfig;
    } else {
        const char* home = std::getenv("HOME");
        configDir = home ? std::string(home) + "/.config" : "/tmp";
    }

    return configDir + "/hypr/hyprlaunch.toml";
}

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

static std::string parseString(const std::string& value) {
    std::string v = trim(value);
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
        return v.substr(1, v.size() - 2);
    }
    return v;
}

static int parseInt(const std::string& value) {
    try {
        return std::stoi(trim(value));
    } catch (...) {
        return 0;
    }
}

Config loadConfig() {
    Config config;

    // Set runtime paths
    const char* homeEnv = std::getenv("HOME");
    std::string home = homeEnv ? homeEnv : "/root";
    config.recentFile = home + "/.cache/hyprlaunch-recent.json";
    config.helpersDir = home + "/.local/bin/helpers";

    std::string configPath = getConfigPath();
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#' || line[0] == '[') {
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));

        if (key == "window_width") config.windowWidth = parseInt(value);
        else if (key == "visible_items") config.visibleItems = parseInt(value);
        else if (key == "hotkey") config.hotkey = parseString(value);
    }

    return config;
}

bool saveConfig(const Config& config) {
    std::string configPath = getConfigPath();

    fs::create_directories(fs::path(configPath).parent_path());

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    file << "# HyprLaunch Configuration\n\n";
    file << "[window]\n";
    file << "window_width = " << config.windowWidth << "\n";
    file << "visible_items = " << config.visibleItems << "\n\n";

    file << "[general]\n";
    file << "hotkey = \"" << config.hotkey << "\"\n";

    return true;
}

} // namespace hyprlaunch
