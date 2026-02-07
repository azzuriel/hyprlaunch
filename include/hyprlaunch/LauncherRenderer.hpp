#pragma once

#include "Forward.hpp"
#include "Config.hpp"
#include "AppEntry.hpp"
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <string>
#include <vector>
#include <atomic>

namespace hyprlaunch {

enum class LauncherMode { Apps, Helpers };

class LauncherRenderer {
public:
    explicit LauncherRenderer(Config& config, AppDiscovery& discovery);
    ~LauncherRenderer();

    void initialize();
    void show();
    void hide();
    void toggle();
    bool isVisible() const;
    void setMode(LauncherMode mode);

private:
    Config& m_config;
    AppDiscovery& m_discovery;

    // GTK widgets
    GtkWidget* m_window      = nullptr;
    GtkWidget* m_searchEntry = nullptr;
    GtkWidget* m_resultsList = nullptr;
    GtkWidget* m_scroll      = nullptr;

    // State
    LauncherMode m_mode = LauncherMode::Apps;
    std::string m_query;
    std::vector<AppEntry> m_results;
    std::string m_calculatorResult;
    int m_selectedIndex = 0;
    std::atomic<bool> m_visible{false};

    // Result buttons for selection tracking
    std::vector<GtkWidget*> m_resultButtons;

    // UI assembly
    void buildUI();

    // Results management
    void updateResults();
    void updateSelection(int newIndex);
    void scrollToIndex(int index);
    void onSearch(const std::string& text);
    void activateSelected();

    // Window positioning
    void centerOnMonitor();

    // Keyboard handler
    static gboolean onKeyPress(GtkEventControllerKey*, guint keyval, guint,
                               GdkModifierType, gpointer data);

    // Helpers
    void removeAllChildren(GtkWidget* box);
};

} // namespace hyprlaunch
