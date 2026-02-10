// HyprLaunch UI - GTK4 layer-shell app launcher
// 1:1 port of AGS ags-launcher-zofi.template

#include "hyprlaunch/LauncherRenderer.hpp"
#include "hyprlaunch/AppDiscovery.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>

namespace hyprlaunch {

// ── CSS — ASE Muted Dark Theme (matching HyprClipX / HyprZones) ─────────────
static const char* LAUNCHER_CSS = R"CSS(
.HyprLaunch {
  background: transparent;
}

.launcher-container {
  background: #0a0a0a;
  border: 1px solid #2a2a2a;
  border-radius: 0;
  padding: 0;
}

.launcher-search {
  background: #121212;
  padding: 10px 14px;
  border-bottom: 1px solid #2a2a2a;
}

.launcher-search-icon {
  font-size: 14px;
  color: #3a6a3a;
}

.launcher-search-input {
  background: transparent;
  border: none;
  color: #8a9a9a;
  font-size: 13px;
  font-family: "Fira Code", monospace;
  caret-color: #3a6a3a;
  outline-color: #3a6a3a;
}

.launcher-search-input:focus {
  outline-color: #3a6a3a;
}

.launcher-scroll {
}

.launcher-list {
  padding: 4px;
}

.launcher-item {
  background: transparent;
  padding: 5px 10px;
  border-radius: 0;
  border: 1px solid transparent;
  margin-bottom: 1px;
  min-height: 32px;
}

.launcher-item:hover {
  background: #1a1a1a;
  border-color: #3a5a3a;
}

.launcher-item.selected {
  background: rgba(42, 90, 42, 0.3);
  border: 1px solid #3a6a3a;
}

.launcher-item.calculator {
  background: rgba(42, 90, 42, 0.2);
  border: 1px solid #3a6a3a;
}

.launcher-item.calculator.selected {
  background: rgba(74, 122, 74, 0.3);
  border-color: #4a7a4a;
}

.launcher-item.calculator .launcher-icon {
  color: #4a7a4a;
  font-size: 18px;
}

.launcher-item.calculator .calc-result {
  color: #4a7a4a;
  font-size: 16px;
}

.launcher-item.recent .launcher-recent-badge {
  font-size: 8px;
  color: #e5c890;
  opacity: 0.8;
}

.launcher-icon-box {
  min-width: 28px;
  min-height: 28px;
}

.launcher-icon-box image {
  min-width: 22px;
  min-height: 22px;
}

.launcher-icon {
  color: #3a6a3a;
  font-size: 18px;
}

.launcher-name {
  font-size: 11px;
  font-weight: 600;
  font-family: "Fira Code", monospace;
  color: #8a9a9a;
}

.launcher-desc {
  font-size: 9px;
  font-family: "Fira Code", monospace;
  color: #6a7a7a;
  margin-top: 1px;
}
)CSS";

// ── Unicode icons ────────────────────────────────────────────────────────────
static const char* ICON_SEARCH     = "\xf0\x9f\x94\x8d";  // Magnifying glass
static const char* ICON_CALCULATOR = "\xf0\x9f\xa7\xae";  // Abacus
static const char* ICON_APP        = "\xf0\x9f\x93\xa6";  // Package
static const char* ICON_RECENT     = "\xf0\x9f\x95\x90";  // Clock

// ============================================================================
// Construction
// ============================================================================

LauncherRenderer::LauncherRenderer(Config& config, AppDiscovery& discovery)
    : m_config(config), m_discovery(discovery) {}

LauncherRenderer::~LauncherRenderer() = default;

// ============================================================================
// Initialization
// ============================================================================

void LauncherRenderer::initialize() {
    // Apply CSS
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css, LAUNCHER_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    // Create window
    m_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(m_window), "HyprLaunch");
    gtk_window_set_decorated(GTK_WINDOW(m_window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);

    gtk_layer_init_for_window(GTK_WINDOW(m_window));
    gtk_layer_set_layer(GTK_WINDOW(m_window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(m_window),
                                 GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
    gtk_window_set_default_size(GTK_WINDOW(m_window),
                                 m_config.windowWidth, m_config.windowHeight());
    gtk_widget_set_size_request(m_window,
                                 m_config.windowWidth, m_config.windowHeight());
    gtk_layer_set_namespace(GTK_WINDOW(m_window), "hyprlaunch");

    gtk_widget_add_css_class(m_window, "HyprLaunch");

    // Prevent close, just hide
    g_signal_connect(m_window, "close-request",
        G_CALLBACK(+[](GtkWindow* win, gpointer) -> gboolean {
            gtk_widget_set_visible(GTK_WIDGET(win), FALSE);
            return TRUE;
        }), nullptr);

    // Build UI widgets
    buildUI();

    // Keyboard handler
    GtkEventController* keyCtrl = gtk_event_controller_key_new();
    g_signal_connect(keyCtrl, "key-pressed", G_CALLBACK(onKeyPress), this);
    gtk_widget_add_controller(m_window, keyCtrl);
}

// ============================================================================
// Show / Hide / Toggle
// ============================================================================

void LauncherRenderer::show() {
    if (!m_window) return;

    // Reset state
    m_query.clear();
    m_selectedIndex = 0;
    m_calculatorResult.clear();

    // Set placeholder based on mode
    if (m_mode == LauncherMode::Apps) {
        gtk_editable_set_text(GTK_EDITABLE(m_searchEntry), "");
        GtkEntryBuffer* buf = gtk_entry_get_buffer(GTK_ENTRY(m_searchEntry));
        (void)buf;
        // GTK4 Entry doesn't have set_placeholder_text as a property easily,
        // so we use the Entry's placeholder via the construct property
        // For simplicity, set it during buildUI and use a label approach
    } else {
        gtk_editable_set_text(GTK_EDITABLE(m_searchEntry), "");
    }

    // Reload data
    if (m_mode == LauncherMode::Apps) {
        m_discovery.reloadApps();
        m_results = m_discovery.searchApps("");
    } else {
        m_discovery.reloadHelpers();
        m_results = m_discovery.searchHelpers("");
    }

    updateResults();

    // Show window
    gtk_widget_set_visible(m_window, TRUE);
    m_visible = true;

    // Reset scroll
    if (m_scroll) {
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(
            GTK_SCROLLED_WINDOW(m_scroll));
        if (vadj) gtk_adjustment_set_value(vadj, 0);
    }
}

void LauncherRenderer::hide() {
    if (!m_window) return;
    gtk_widget_set_visible(m_window, FALSE);
    m_visible = false;
}

void LauncherRenderer::toggle() {
    if (m_visible) {
        hide();
    } else {
        show();
    }
}

bool LauncherRenderer::isVisible() const {
    return m_visible;
}

void LauncherRenderer::setMode(LauncherMode mode) {
    m_mode = mode;
}

// ============================================================================
// Build UI
// ============================================================================

void LauncherRenderer::buildUI() {
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(mainBox, "launcher-container");

    // ── Search bar ──
    GtkWidget* searchBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(searchBox, "launcher-search");

    GtkWidget* searchIcon = gtk_label_new(ICON_SEARCH);
    gtk_widget_add_css_class(searchIcon, "launcher-search-icon");

    m_searchEntry = gtk_entry_new();
    gtk_widget_set_hexpand(m_searchEntry, TRUE);
    gtk_widget_set_can_focus(m_searchEntry, FALSE);
    gtk_widget_add_css_class(m_searchEntry, "launcher-search-input");
    gtk_entry_set_placeholder_text(GTK_ENTRY(m_searchEntry),
                                    "Search apps... (= for calculator)");

    g_signal_connect(m_searchEntry, "changed",
        G_CALLBACK(+[](GtkEditable* editable, gpointer data) {
            auto* self = static_cast<LauncherRenderer*>(data);
            const char* text = gtk_editable_get_text(editable);
            self->onSearch(text ? text : "");
        }), this);

    gtk_box_append(GTK_BOX(searchBox), searchIcon);
    gtk_box_append(GTK_BOX(searchBox), m_searchEntry);

    // ── Results area ──
    m_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_scroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(m_scroll, TRUE);
    gtk_widget_add_css_class(m_scroll, "launcher-scroll");

    m_resultsList = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(m_resultsList, "launcher-list");

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_scroll), m_resultsList);

    // ── Assemble ──
    gtk_box_append(GTK_BOX(mainBox), searchBox);
    gtk_box_append(GTK_BOX(mainBox), m_scroll);

    gtk_window_set_child(GTK_WINDOW(m_window), mainBox);
}

// ============================================================================
// Search Handler
// ============================================================================

void LauncherRenderer::onSearch(const std::string& text) {
    m_query = text;
    m_selectedIndex = 0;
    m_calculatorResult.clear();

    if (m_mode == LauncherMode::Apps && !text.empty() && text[0] == '=') {
        m_calculatorResult = AppDiscovery::evaluateCalculator(text);
        m_results.clear();
    } else {
        if (m_mode == LauncherMode::Apps) {
            m_results = m_discovery.searchApps(text);
        } else {
            m_results = m_discovery.searchHelpers(text);
        }
    }

    updateResults();

    // Reset scroll to top
    if (m_scroll) {
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(
            GTK_SCROLLED_WINDOW(m_scroll));
        if (vadj) gtk_adjustment_set_value(vadj, 0);
    }
}

// ============================================================================
// Results Rendering
// ============================================================================

void LauncherRenderer::removeAllChildren(GtkWidget* box) {
    GtkWidget* child = gtk_widget_get_first_child(box);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(box), child);
        child = next;
    }
}

void LauncherRenderer::updateResults() {
    if (!m_resultsList) return;

    removeAllChildren(m_resultsList);
    m_resultButtons.clear();

    // ── Calculator result row ──
    if (!m_calculatorResult.empty()) {
        GtkWidget* calcRow = gtk_button_new();
        gtk_widget_set_can_focus(calcRow, FALSE);
        gtk_widget_add_css_class(calcRow, "launcher-item");
        gtk_widget_add_css_class(calcRow, "calculator");
        if (m_selectedIndex == 0) gtk_widget_add_css_class(calcRow, "selected");

        GtkWidget* calcBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

        GtkWidget* calcIcon = gtk_label_new(ICON_CALCULATOR);
        gtk_widget_add_css_class(calcIcon, "launcher-icon");

        GtkWidget* calcContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_hexpand(calcContent, TRUE);

        GtkWidget* calcValue = gtk_label_new(m_calculatorResult.c_str());
        gtk_label_set_xalign(GTK_LABEL(calcValue), 0);
        gtk_widget_add_css_class(calcValue, "launcher-name");
        gtk_widget_add_css_class(calcValue, "calc-result");

        GtkWidget* calcHint = gtk_label_new("Enter to copy result");
        gtk_label_set_xalign(GTK_LABEL(calcHint), 0);
        gtk_widget_add_css_class(calcHint, "launcher-desc");

        gtk_box_append(GTK_BOX(calcContent), calcValue);
        gtk_box_append(GTK_BOX(calcContent), calcHint);

        gtk_box_append(GTK_BOX(calcBox), calcIcon);
        gtk_box_append(GTK_BOX(calcBox), calcContent);

        gtk_button_set_child(GTK_BUTTON(calcRow), calcBox);

        // Click handler
        std::string* resultCopy = new std::string(m_calculatorResult);
        g_signal_connect_data(calcRow, "clicked",
            G_CALLBACK(+[](GtkButton*, gpointer data) {
                auto* result = static_cast<std::string*>(data);
                AppDiscovery::copyToClipboard(*result);
            }), resultCopy,
            +[](gpointer data, GClosure*) {
                delete static_cast<std::string*>(data);
            }, G_CONNECT_DEFAULT);

        gtk_box_append(GTK_BOX(m_resultsList), calcRow);
        m_resultButtons.push_back(calcRow);
    }

    // ── App result rows ──
    for (size_t i = 0; i < m_results.size(); i++) {
        const auto& entry = m_results[i];
        int actualIndex = !m_calculatorResult.empty()
                          ? static_cast<int>(i) + 1
                          : static_cast<int>(i);

        GtkWidget* row = gtk_button_new();
        gtk_widget_set_can_focus(row, FALSE);
        gtk_widget_add_css_class(row, "launcher-item");
        if (actualIndex == m_selectedIndex) {
            gtk_widget_add_css_class(row, "selected");
        }

        // Check if recent (only in apps mode with empty query)
        bool isRecent = false;
        if (m_mode == LauncherMode::Apps && m_query.empty()) {
            // Recent apps are at the top of the list when query is empty
            // We mark the first few items as recent based on their position
            // (the search function puts recent apps first)
            isRecent = (i < 5);  // Approximate; proper check would need recent list access
        }
        if (isRecent) {
            gtk_widget_add_css_class(row, "recent");
        }

        GtkWidget* rowBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

        // ── Icon ──
        GtkWidget* iconWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_valign(iconWidget, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(iconWidget, "launcher-icon-box");

        if (!entry.icon.empty()) {
            GtkWidget* image = gtk_image_new_from_icon_name(entry.icon.c_str());
            gtk_image_set_pixel_size(GTK_IMAGE(image), 36);
            gtk_box_append(GTK_BOX(iconWidget), image);
        } else {
            GtkWidget* fallback = gtk_label_new(ICON_APP);
            gtk_widget_add_css_class(fallback, "launcher-icon");
            gtk_box_append(GTK_BOX(iconWidget), fallback);
        }

        // ── Content ──
        GtkWidget* content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_hexpand(content, TRUE);
        gtk_widget_set_valign(content, GTK_ALIGN_CENTER);

        GtkWidget* nameRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

        GtkWidget* nameLabel = gtk_label_new(entry.name.c_str());
        gtk_label_set_xalign(GTK_LABEL(nameLabel), 0);
        gtk_label_set_ellipsize(GTK_LABEL(nameLabel), PANGO_ELLIPSIZE_END);
        gtk_widget_add_css_class(nameLabel, "launcher-name");
        gtk_box_append(GTK_BOX(nameRow), nameLabel);

        if (isRecent) {
            GtkWidget* recentBadge = gtk_label_new(ICON_RECENT);
            gtk_widget_add_css_class(recentBadge, "launcher-recent-badge");
            gtk_box_append(GTK_BOX(nameRow), recentBadge);
        }

        gtk_box_append(GTK_BOX(content), nameRow);

        if (!entry.description.empty()) {
            GtkWidget* desc = gtk_label_new(entry.description.c_str());
            gtk_label_set_xalign(GTK_LABEL(desc), 0);
            gtk_label_set_wrap(GTK_LABEL(desc), TRUE);
            gtk_label_set_ellipsize(GTK_LABEL(desc), PANGO_ELLIPSIZE_END);
            gtk_label_set_max_width_chars(GTK_LABEL(desc), 60);
            gtk_widget_add_css_class(desc, "launcher-desc");
            gtk_box_append(GTK_BOX(content), desc);
        }

        gtk_box_append(GTK_BOX(rowBox), iconWidget);
        gtk_box_append(GTK_BOX(rowBox), content);

        gtk_button_set_child(GTK_BUTTON(row), rowBox);

        // Click handler — capture index
        struct ClickData {
            LauncherRenderer* self;
            size_t index;
        };
        auto* clickData = new ClickData{this, i};
        g_signal_connect_data(row, "clicked",
            G_CALLBACK(+[](GtkButton*, gpointer data) {
                auto* cd = static_cast<ClickData*>(data);
                auto* self = cd->self;
                size_t idx = cd->index;
                if (idx < self->m_results.size()) {
                    if (self->m_mode == LauncherMode::Apps) {
                        self->m_discovery.launchApp(self->m_results[idx]);
                    } else {
                        self->m_discovery.launchHelper(self->m_results[idx]);
                    }
                    self->hide();
                }
            }), clickData,
            +[](gpointer data, GClosure*) {
                delete static_cast<ClickData*>(data);
            }, G_CONNECT_DEFAULT);

        gtk_box_append(GTK_BOX(m_resultsList), row);
        m_resultButtons.push_back(row);
    }
}

// ============================================================================
// Selection Management
// ============================================================================

void LauncherRenderer::updateSelection(int newIndex) {
    int maxIndex = static_cast<int>(m_resultButtons.size()) - 1;
    if (maxIndex < 0) return;

    int clampedIndex = std::max(0, std::min(newIndex, maxIndex));

    for (int i = 0; i <= maxIndex; i++) {
        if (i == clampedIndex) {
            gtk_widget_add_css_class(m_resultButtons[i], "selected");
        } else {
            gtk_widget_remove_css_class(m_resultButtons[i], "selected");
        }
    }

    m_selectedIndex = clampedIndex;
    scrollToIndex(clampedIndex);
}

void LauncherRenderer::scrollToIndex(int index) {
    if (!m_scroll || index < 0 || index >= static_cast<int>(m_resultButtons.size()))
        return;

    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(
        GTK_SCROLLED_WINDOW(m_scroll));
    if (!vadj) return;

    GtkWidget* btn = m_resultButtons[index];
    graphene_rect_t bounds;
    if (!gtk_widget_compute_bounds(btn, m_resultsList, &bounds)) return;

    double itemTop = bounds.origin.y;
    double itemHeight = bounds.size.height;
    double itemBottom = itemTop + itemHeight;

    double scrollY = gtk_adjustment_get_value(vadj);
    double scrollHeight = gtk_adjustment_get_page_size(vadj);

    if (itemBottom > scrollY + scrollHeight) {
        gtk_adjustment_set_value(vadj, itemBottom - scrollHeight + 8);
    } else if (itemTop < scrollY) {
        gtk_adjustment_set_value(vadj, itemTop - 8);
    }
}

// ============================================================================
// Activate Selected
// ============================================================================

void LauncherRenderer::activateSelected() {
    if (!m_calculatorResult.empty() && m_selectedIndex == 0) {
        AppDiscovery::copyToClipboard(m_calculatorResult);
        hide();
        return;
    }

    int appIndex = !m_calculatorResult.empty()
                   ? m_selectedIndex - 1
                   : m_selectedIndex;

    if (appIndex >= 0 && appIndex < static_cast<int>(m_results.size())) {
        if (m_mode == LauncherMode::Apps) {
            m_discovery.launchApp(m_results[appIndex]);
        } else {
            m_discovery.launchHelper(m_results[appIndex]);
        }
        hide();
    }
}

// ============================================================================
// Keyboard Handler
// ============================================================================

gboolean LauncherRenderer::onKeyPress(GtkEventControllerKey*, guint keyval,
                                       guint, GdkModifierType, gpointer data) {
    auto* self = static_cast<LauncherRenderer*>(data);
    int maxIndex = static_cast<int>(self->m_resultButtons.size()) - 1;

    switch (keyval) {
        case GDK_KEY_Escape:
            self->hide();
            return TRUE;

        case GDK_KEY_Down:
            self->updateSelection(std::min(self->m_selectedIndex + 1, maxIndex));
            return TRUE;

        case GDK_KEY_Up:
            self->updateSelection(std::max(self->m_selectedIndex - 1, 0));
            return TRUE;

        case GDK_KEY_Page_Down:
            self->updateSelection(std::min(self->m_selectedIndex + 5, maxIndex));
            return TRUE;

        case GDK_KEY_Page_Up:
            self->updateSelection(std::max(self->m_selectedIndex - 5, 0));
            return TRUE;

        case GDK_KEY_Home:
            self->updateSelection(0);
            return TRUE;

        case GDK_KEY_End:
            self->updateSelection(maxIndex);
            return TRUE;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            self->activateSelected();
            return TRUE;

        case GDK_KEY_Tab:
            if (maxIndex >= 0) {
                self->updateSelection((self->m_selectedIndex + 1) % (maxIndex + 1));
            }
            return TRUE;

        case GDK_KEY_BackSpace: {
            const char* t = gtk_editable_get_text(GTK_EDITABLE(self->m_searchEntry));
            std::string cur = t ? t : "";
            if (!cur.empty()) {
                cur.pop_back();
                gtk_editable_set_text(GTK_EDITABLE(self->m_searchEntry), cur.c_str());
            }
            return TRUE;
        }

        default: {
            guint32 ch = gdk_keyval_to_unicode(keyval);
            if (ch > 31 && ch < 127) {
                const char* t = gtk_editable_get_text(GTK_EDITABLE(self->m_searchEntry));
                std::string cur = t ? t : "";
                cur += static_cast<char>(ch);
                gtk_editable_set_text(GTK_EDITABLE(self->m_searchEntry), cur.c_str());
                return TRUE;
            }
            return FALSE;
        }
    }
}

} // namespace hyprlaunch
