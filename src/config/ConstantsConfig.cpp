#include "config/ConstantsConfig.h"
#include "config/ConfigManager.h"
#include "flatui/FlatUIConstants.h"
#include <sstream>

ConstantsConfig& ConstantsConfig::getInstance() {
    static ConstantsConfig instance;
    return instance;
}

ConstantsConfig::ConstantsConfig()
    : primaryFrameBorderColour(FLATUI_PRIMARY_FRAME_BORDER_COLOUR),
      primaryContentBgColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR),
      barTopMargin(FLATUI_BAR_TOP_MARGIN),
      defaultFontFaceName(FLATUI_DEFAULT_FONT_FACE_NAME),
      defaultFontSize(FLATUI_DEFAULT_FONT_SIZE)
{
}

void ConstantsConfig::initialize(ConfigManager& config) {
    auto readColour = [&](const std::string& key, const wxColour& def) {
        std::string defaultStr = std::to_string(def.Red()) + "," +
                                 std::to_string(def.Green()) + "," +
                                 std::to_string(def.Blue());
        std::string val = config.getString("FlatUIConstants", key, defaultStr);
        std::istringstream ss(val);
        int r, g, b;
        char comma;
        if (ss >> r >> comma >> g >> comma >> b) {
            return wxColour(r, g, b);
        }
        return def;
    };

    primaryFrameBorderColour = readColour("PrimaryFrameBorderColour", FLATUI_PRIMARY_FRAME_BORDER_COLOUR);
    primaryContentBgColour = readColour("PrimaryContentBgColour", FLATUI_PRIMARY_CONTENT_BG_COLOUR);

    barTopMargin = config.getInt("FlatUIConstants", "BarTopMargin", FLATUI_BAR_TOP_MARGIN);

    std::string face = config.getString("FlatUIConstants", "DefaultFontFaceName",
                                        FLATUI_DEFAULT_FONT_FACE_NAME.ToStdString());
    defaultFontFaceName = wxString(face);

    defaultFontSize = config.getInt("FlatUIConstants", "DefaultFontSize", FLATUI_DEFAULT_FONT_SIZE);

    {
        auto keys = config.getKeys("FlatUIConstants");
        for (const auto& key : keys) {
            configMap[key] = config.getString("FlatUIConstants", key, "");
        }
    }
}

const wxColour& ConstantsConfig::getPrimaryFrameBorderColour() const {
    return primaryFrameBorderColour;
}

const wxColour& ConstantsConfig::getPrimaryContentBgColour() const {
    return primaryContentBgColour;
}

int ConstantsConfig::getBarTopMargin() const {
    return barTopMargin;
}

const wxString& ConstantsConfig::getDefaultFontFaceName() const {
    return defaultFontFaceName;
}

int ConstantsConfig::getDefaultFontSize() const {
    return defaultFontSize;
}

std::string ConstantsConfig::getStringValue(const std::string& key, const std::string& defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end() && !it->second.empty()) return it->second;
    return defaultValue;
}

int ConstantsConfig::getIntValue(const std::string& key, int defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try { return std::stoi(it->second); } catch (...) {}
    }
    return defaultValue;
}

double ConstantsConfig::getDoubleValue(const std::string& key, double defaultValue) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try { return std::stod(it->second); } catch (...) {}
    }
    return defaultValue;
}

wxColour ConstantsConfig::getColourValue(const std::string& key, const wxColour& defaultValue) const {
    std::string val = getStringValue(key, "");
    if (!val.empty()) {
        std::istringstream ss(val);
        int r, g, b;
        char comma;
        if (ss >> r >> comma >> g >> comma >> b) {
            return wxColour(r, g, b);
        }
    }
    return defaultValue;
} 