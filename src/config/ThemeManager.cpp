#include "config/ThemeManager.h"
#include "config/SvgIconManager.h"
#include "logger/Logger.h"
#include <wx/settings.h>
#include <sstream>
#include <set>

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
    // Load theme configurations from new format
    if (m_configManager) {
        // Load theme colors from ThemeColors section
        auto colorKeys = m_configManager->getKeys("ThemeColors");
        
        // Initialize three themes
        ThemeProfile defaultTheme, darkTheme, blueTheme;
        
        defaultTheme.name = "default";
        defaultTheme.displayName = "Default Light";
        darkTheme.name = "dark"; 
        darkTheme.displayName = "Dark";
        blueTheme.name = "blue";
        blueTheme.displayName = "Modern Blue";
        
        // Parse theme colors in new format: default_value;dark_value;blue_value
        for (const auto& key : colorKeys) {
            std::string value = m_configManager->getString("ThemeColors", key, "");
            if (!value.empty()) {
                // Split by semicolon to get three theme values
                std::vector<std::string> themeValues = splitString(value, ';');
                
                if (themeValues.size() >= 3) {
                    // Parse each theme's color value
                    wxColour defaultColor = parseColour(themeValues[0]);
                    wxColour darkColor = parseColour(themeValues[1]);
                    wxColour blueColor = parseColour(themeValues[2]);
                    
                    if (defaultColor.IsOk()) defaultTheme.colours[key] = defaultColor;
                    if (darkColor.IsOk()) darkTheme.colours[key] = darkColor;
                    if (blueColor.IsOk()) blueTheme.colours[key] = blueColor;
                }
            }
        }
        
        // Load SVG theme colors
        auto svgKeys = m_configManager->getKeys("SvgTheme");
        for (const auto& key : svgKeys) {
            if (key == "SvgThemeEnabled") {
                int enabled = m_configManager->getInt("SvgTheme", key, 1);
                defaultTheme.integers[key] = enabled;
                darkTheme.integers[key] = enabled;
                blueTheme.integers[key] = enabled;
            } else {
                std::string value = m_configManager->getString("SvgTheme", key, "");
                if (!value.empty()) {
                    std::vector<std::string> themeValues = splitString(value, ';');
                    if (themeValues.size() >= 3) {
                        wxColour defaultColor = parseColour(themeValues[0]);
                        wxColour darkColor = parseColour(themeValues[1]);
                        wxColour blueColor = parseColour(themeValues[2]);
                        
                        if (defaultColor.IsOk()) defaultTheme.colours[key] = defaultColor;
                        if (darkColor.IsOk()) darkTheme.colours[key] = darkColor;
                        if (blueColor.IsOk()) blueTheme.colours[key] = blueColor;
                    }
                }
            }
        }
        
        // Load size configurations (same for all themes)
        loadSizeConfigurations(defaultTheme);
        loadSizeConfigurations(darkTheme);
        loadSizeConfigurations(blueTheme);
        
        // Set default fonts
        defaultTheme.defaultFont = loadFont();
        darkTheme.defaultFont = loadFont();
        blueTheme.defaultFont = loadFont();
        
        m_themes["default"] = defaultTheme;
        m_themes["dark"] = darkTheme;
        m_themes["blue"] = blueTheme;
        
        LOG_INF("Loaded themes with new configuration format", "ThemeManager");
    } else {
        LOG_ERR("ConfigManager not available for loading themes", "ThemeManager");
    }
}

std::vector<std::string> ThemeManager::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        tokens.push_back(token);
    }
    
    return tokens;
}

void ThemeManager::loadSizeConfigurations(ThemeProfile& theme) {
    if (!m_configManager) return;
    
    // Load size configurations from various sections
    std::vector<std::string> sizeSections = {
        "BarSizes", "ButtonBarSizes", "Separators", "Icons", 
        "GallerySizes", "PanelSizes", "HomeSpace", "HomeMenu"
    };
    
    for (const auto& section : sizeSections) {
        auto keys = m_configManager->getKeys(section);
        for (const auto& key : keys) {
            int value = m_configManager->getInt(section, key, 0);
            if (value != 0) {
                theme.integers[key] = value;
            }
        }
    }
}

wxFont ThemeManager::loadFont() {
    if (!m_configManager) {
        return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    }
    
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    
    int fontSize = m_configManager->getInt("Font", "DefaultFontSize", 8);
    std::string fontName = m_configManager->getString("Font", "DefaultFontFaceName", "Consolas");
    
    font.SetPointSize(fontSize);
    font.SetFaceName(fontName);
    
    return font;
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
        themes.push_back(pair.first); // Return theme name, not display name
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
        // Only log warning once per key to avoid log spam
        static std::set<std::string> loggedKeys;
        if (loggedKeys.find(key) == loggedKeys.end()) {
            LOG_WRN("Integer key not found: " + key + " in theme: " + m_currentTheme, "ThemeManager");
            loggedKeys.insert(key);
        }
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