#include "config/ThemeManager.h"
#include "config/SvgIconManager.h"
#include "logger/Logger.h"
#include <wx/settings.h>
#include <sstream>

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() 
    : m_configManager(nullptr), m_currentTheme("default"), m_initialized(false) {
}

ThemeManager::~ThemeManager() {
}

void ThemeManager::initialize(ConfigManager& config) {
    if (m_initialized) {
        LOG_WRN("Theme manager already initialized", "ThemeManager");
        return;
    }
    
    m_configManager = &config;
    
    // Load built-in themes
    loadBuiltinThemes();
    
    // Load current theme from config
    std::string currentTheme = m_configManager->getString("Theme", "CurrentTheme", "default");
    if (!setCurrentTheme(currentTheme)) {
        LOG_WRN("Failed to load theme: " + currentTheme + ", using default", "ThemeManager");
        setCurrentTheme("default");
    }
    
    m_initialized = true;
    LOG_INF("Theme manager initialized with theme: " + m_currentTheme, "ThemeManager");
}

void ThemeManager::loadBuiltinThemes() {
    // Default theme - based on current FlatUIConstants
    ThemeProfile defaultTheme;
    defaultTheme.name = "default";
    defaultTheme.displayName = "Default Light";
    
    // Load colors from existing config structure
    if (m_configManager) {
        auto keys = m_configManager->getKeys("FlatUIConstants");
        for (const auto& key : keys) {
            std::string value = m_configManager->getString("FlatUIConstants", key, "");
            if (!value.empty()) {
                // Try to parse as color first
                wxColour color = parseColour(value);
                if (color.IsOk()) {
                    defaultTheme.colours[key] = color;
                } else {
                    // Try to parse as integer
                    try {
                        int intValue = std::stoi(value);
                        defaultTheme.integers[key] = intValue;
                    } catch (...) {
                        // Store as string
                        defaultTheme.strings[key] = value;
                    }
                }
            }
        }
    }
    
    // Set default font
    defaultTheme.defaultFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    defaultTheme.defaultFont.SetPointSize(8);
    defaultTheme.defaultFont.SetFaceName("Consolas");
    
    // Add SVG theme settings to default theme
    defaultTheme.colours["SvgPrimaryIconColour"] = wxColour(100, 100, 100);
    defaultTheme.colours["SvgSecondaryIconColour"] = wxColour(70, 70, 70);
    defaultTheme.colours["SvgDisabledIconColour"] = wxColour(180, 180, 180);
    defaultTheme.colours["SvgHighlightIconColour"] = wxColour(0, 120, 215);
    defaultTheme.integers["SvgThemeEnabled"] = 1;
    
    // Dark theme
    ThemeProfile darkTheme = defaultTheme;
    darkTheme.name = "dark";
    darkTheme.displayName = "Dark";
    
    // Override colors for dark theme
    darkTheme.colours["WindowMotionColor"] = wxColour(60, 60, 60);
    darkTheme.colours["FrameBorderColor"] = wxColour(80, 80, 80);
    darkTheme.colours["PrimaryFrameBorderColour"] = wxColour(100, 100, 100);
    darkTheme.colours["PrimaryContentBgColour"] = wxColour(45, 45, 48);
    darkTheme.colours["DefaultBgColour"] = wxColour(37, 37, 38);
    darkTheme.colours["DefaultBorderColour"] = wxColour(85, 85, 85);
    darkTheme.colours["DefaultTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["HighlightColour"] = wxColour(0, 122, 204);
    
    // Bar colors for dark theme
    darkTheme.colours["BarBackgroundColour"] = wxColour(45, 45, 48);
    darkTheme.colours["BarBorderColour"] = wxColour(85, 85, 85);
    darkTheme.colours["ActBarBackgroundColour"] = wxColour(37, 37, 38);
    darkTheme.colours["BarTabBorderColour"] = wxColour(85, 85, 85);
    darkTheme.colours["BarActiveTabTopBorderColour"] = wxColour(100, 100, 100);
    darkTheme.colours["BarActiveTabBgColour"] = wxColour(37, 37, 38);
    darkTheme.colours["BarTabBorderTopColour"] = wxColour(0, 122, 204);
    darkTheme.colours["BarActiveTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["BarInactiveTextColour"] = wxColour(170, 170, 170);
    
    // Panel colors for dark theme
    darkTheme.colours["PanelBorderColour"] = wxColour(85, 85, 85);
    darkTheme.colours["PanelHeaderColour"] = wxColour(55, 55, 58);
    darkTheme.colours["PanelHeaderTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["PanelSepatatorColor"] = wxColour(85, 85, 85);
    
    // Button colors for dark theme
    darkTheme.colours["ButtonbarDefaultBgColour"] = wxColour(45, 45, 48);
    darkTheme.colours["ButtonbarDefaultHoverBgColour"] = wxColour(62, 62, 64);
    darkTheme.colours["ButtonbarDefaultPressedBgColour"] = wxColour(85, 85, 85);
    darkTheme.colours["ButtonbarDefaultTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["ButtonbarDefaultBorderColour"] = wxColour(85, 85, 85);
    
    // Home space colors for dark theme
    darkTheme.colours["HomespaceHoverBgColour"] = wxColour(62, 62, 64);
    
    // SVG icon colors for dark theme
    darkTheme.colours["SvgPrimaryIconColour"] = wxColour(220, 220, 220);
    darkTheme.colours["SvgSecondaryIconColour"] = wxColour(170, 170, 170);
    darkTheme.colours["SvgDisabledIconColour"] = wxColour(120, 120, 120);
    darkTheme.colours["SvgHighlightIconColour"] = wxColour(0, 122, 204);
    darkTheme.integers["SvgThemeEnabled"] = 1;
    
    // Text colors for dark theme
    darkTheme.colours["MenuTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["ErrorTextColour"] = wxColour(244, 71, 71);
    darkTheme.colours["PlaceholderTextColour"] = wxColour(140, 140, 140);
    
    // Background colors for dark theme
    darkTheme.colours["MainBackgroundColour"] = wxColour(45, 45, 48);
    darkTheme.colours["SecondaryBackgroundColour"] = wxColour(37, 37, 38);
    darkTheme.colours["ScrolledWindowBgColour"] = wxColour(37, 37, 38);
    darkTheme.colours["IconPanelBgColour"] = wxColour(37, 37, 38);
    darkTheme.colours["SvgPanelBgColour"] = wxColour(45, 45, 48);
    darkTheme.colours["FrameAppWorkspaceColour"] = wxColour(30, 30, 30);
    
    // System button colors for dark theme
    darkTheme.colours["SystemButtonTextColour"] = wxColour(220, 220, 220);
    darkTheme.colours["SystemButtonHoverTextColour"] = wxColour(255, 255, 255);
    darkTheme.colours["SystemButtonBgColour"] = wxColour(45, 45, 48);
    darkTheme.colours["SystemButtonCloseHoverColour"] = wxColour(196, 43, 28);
    
    // Dropdown colors for dark theme
    darkTheme.colours["DropdownBackgroundColour"] = wxColour(45, 45, 48);
    darkTheme.colours["DropdownBorderColour"] = wxColour(85, 85, 85);
    darkTheme.colours["DropdownHoverColour"] = wxColour(62, 62, 64);
    darkTheme.colours["DropdownSelectionBgColour"] = wxColour(0, 122, 204);
    darkTheme.colours["DropdownSelectionTextColour"] = wxColour(255, 255, 255);
    
    // Blue theme - modern blue accent
    ThemeProfile blueTheme = defaultTheme;
    blueTheme.name = "blue";
    blueTheme.displayName = "Modern Blue";
    
    blueTheme.colours["HighlightColour"] = wxColour(0, 120, 215);
    blueTheme.colours["BarTabBorderTopColour"] = wxColour(0, 120, 215);
    blueTheme.colours["PrimaryFrameBorderColour"] = wxColour(0, 120, 215);
    blueTheme.colours["FrameBorderColor"] = wxColour(0, 120, 215);
    
    // SVG icon colors for blue theme (same as default but with blue highlights)
    blueTheme.colours["SvgPrimaryIconColour"] = wxColour(100, 100, 100);
    blueTheme.colours["SvgSecondaryIconColour"] = wxColour(70, 70, 70);
    blueTheme.colours["SvgDisabledIconColour"] = wxColour(180, 180, 180);
    blueTheme.colours["SvgHighlightIconColour"] = wxColour(0, 120, 215);
    blueTheme.integers["SvgThemeEnabled"] = 1;
    
    m_themes["default"] = defaultTheme;
    m_themes["dark"] = darkTheme;
    m_themes["blue"] = blueTheme;
}

wxColour ThemeManager::parseColour(const std::string& value) const {
    if (value.empty()) return wxColour();
    
    std::istringstream ss(value);
    int r, g, b;
    char comma;
    
    if (ss >> r >> comma >> g >> comma >> b) {
        if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
            return wxColour(r, g, b);
        }
    }
    
    return wxColour();
}

bool ThemeManager::setCurrentTheme(const std::string& themeName) {
    auto it = m_themes.find(themeName);
    if (it == m_themes.end()) {
        LOG_ERR("Theme not found: " + themeName, "ThemeManager");
        return false;
    }
    
    m_currentTheme = themeName;
    
    // Save current theme to config
    if (m_configManager) {
        m_configManager->setString("Theme", "CurrentTheme", themeName);
        m_configManager->save();
    }
    
    // Notify listeners
    notifyThemeChange();
    
    LOG_INF("Theme changed to: " + themeName, "ThemeManager");
    return true;
}

std::vector<std::string> ThemeManager::getAvailableThemes() const {
    std::vector<std::string> themes;
    for (const auto& pair : m_themes) {
        themes.push_back(pair.second.displayName);
    }
    return themes;
}

std::string ThemeManager::getCurrentTheme() const {
    return m_currentTheme;
}

wxColour ThemeManager::getColour(const std::string& key) const {
    if (!m_initialized) {
        LOG_ERR("Theme manager not initialized", "ThemeManager");
        return wxColour(255, 0, 0); // Error color
    }
    
    auto themeIt = m_themes.find(m_currentTheme);
    if (themeIt == m_themes.end()) {
        LOG_ERR("Current theme not found: " + m_currentTheme, "ThemeManager");
        return wxColour(255, 0, 0);
    }
    
    auto colorIt = themeIt->second.colours.find(key);
    if (colorIt == themeIt->second.colours.end()) {
        LOG_WRN("Color key not found: " + key + " in theme: " + m_currentTheme, "ThemeManager");
        return wxColour(255, 0, 0);
    }
    
    return colorIt->second;
}

int ThemeManager::getInt(const std::string& key) const {
    if (!m_initialized) {
        LOG_ERR("Theme manager not initialized", "ThemeManager");
        return -1;
    }
    
    auto themeIt = m_themes.find(m_currentTheme);
    if (themeIt == m_themes.end()) {
        LOG_ERR("Current theme not found: " + m_currentTheme, "ThemeManager");
        return -1;
    }
    
    auto intIt = themeIt->second.integers.find(key);
    if (intIt == themeIt->second.integers.end()) {
        LOG_WRN("Integer key not found: " + key + " in theme: " + m_currentTheme, "ThemeManager");
        return -1;
    }
    
    return intIt->second;
}

std::string ThemeManager::getString(const std::string& key) const {
    if (!m_initialized) {
        LOG_ERR("Theme manager not initialized", "ThemeManager");
        return "";
    }
    
    auto themeIt = m_themes.find(m_currentTheme);
    if (themeIt == m_themes.end()) {
        LOG_ERR("Current theme not found: " + m_currentTheme, "ThemeManager");
        return "";
    }
    
    auto strIt = themeIt->second.strings.find(key);
    if (strIt == themeIt->second.strings.end()) {
        LOG_WRN("String key not found: " + key + " in theme: " + m_currentTheme, "ThemeManager");
        return "";
    }
    
    return strIt->second;
}

wxFont ThemeManager::getDefaultFont() const {
    if (!m_initialized) {
        return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    }
    
    auto themeIt = m_themes.find(m_currentTheme);
    if (themeIt == m_themes.end()) {
        return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    }
    
    return themeIt->second.defaultFont;
}

void ThemeManager::addThemeChangeListener(void* listener, std::function<void()> callback) {
    m_listeners[listener] = callback;
}

void ThemeManager::removeThemeChangeListener(void* listener) {
    m_listeners.erase(listener);
}

void ThemeManager::notifyThemeChange() {
    // Clear SVG theme cache when theme changes
    try {
        SvgIconManager::GetInstance().ClearThemeCache();
        LOG_INF("SVG theme cache cleared on theme change", "ThemeManager");
    } catch (const std::exception& e) {
        LOG_ERR(wxString::Format("Error clearing SVG theme cache: %s", e.what()).ToStdString(), "ThemeManager");
    } catch (...) {
        LOG_ERR("Unknown error clearing SVG theme cache", "ThemeManager");
    }
    
    // Notify other listeners
    for (const auto& pair : m_listeners) {
        try {
            pair.second();
        } catch (...) {
            LOG_ERR("Error in theme change listener", "ThemeManager");
        }
    }
}

bool ThemeManager::saveCurrentTheme() {
    if (!m_configManager) return false;
    return m_configManager->save();
}

bool ThemeManager::reloadThemes() {
    if (!m_configManager) return false;
    
    loadBuiltinThemes();
    return setCurrentTheme(m_currentTheme);
} 