#pragma once
#include <string>

namespace hyprlaunch {

struct Config {
    int windowWidth = 530;
    int visibleItems = 20;

    std::string hotkey = "SUPER D";

    // Paths (set at runtime from $HOME)
    std::string recentFile;   // ~/.cache/hyprlaunch-recent.json
    std::string helpersDir;   // ~/.local/bin/helpers

    // Computed from visibleItems (search bar + items + padding)
    static constexpr int SEARCH_HEIGHT = 52;  // padding 10+10 + entry ~30 + border 1 + 1 extra
    static constexpr int ITEM_HEIGHT = 45;    // 32 min-height + 5+5 padding + 2 border + 1 margin
    static constexpr int CHROME = 9;          // list padding 4+4 + container border 1+1 - 1 adjust

    int windowHeight() const {
        return SEARCH_HEIGHT + (visibleItems * ITEM_HEIGHT) + CHROME;
    }
};

} // namespace hyprlaunch
