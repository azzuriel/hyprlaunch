#pragma once

#include "Config.hpp"
#include <string>

namespace hyprlaunch {

Config loadConfig();
bool saveConfig(const Config& config);
std::string getConfigPath();

} // namespace hyprlaunch
