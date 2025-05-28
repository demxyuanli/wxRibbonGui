#ifndef CONSTANTS_CONFIG_H
#define CONSTANTS_CONFIG_H

#include "config/ConfigManager.h"
#include <wx/colour.h>
#include <wx/string.h>
#include <map>
#include <vector>
#include <string>

class ConstantsConfig {
public:
    static ConstantsConfig& getInstance();

    void initialize(ConfigManager& config);

    const wxColour& getPrimaryFrameBorderColour() const;
    const wxColour& getPrimaryContentBgColour() const;
    int getBarTopMargin() const;
    const wxString& getDefaultFontFaceName() const;
    int getDefaultFontSize() const;

    std::string getStringValue(const std::string& key, const std::string& defaultValue = "") const;
    int getIntValue(const std::string& key, int defaultValue) const;
    double getDoubleValue(const std::string& key, double defaultValue) const;
    wxColour getColourValue(const std::string& key, const wxColour& defaultValue) const;

private:
    ConstantsConfig();
    ConstantsConfig(const ConstantsConfig&) = delete;
    ConstantsConfig& operator=(const ConstantsConfig&) = delete;

    wxColour primaryFrameBorderColour;
    wxColour primaryContentBgColour;
    int barTopMargin;
    wxString defaultFontFaceName;
    int defaultFontSize;

    std::map<std::string, std::string> configMap;
};

#endif // CONSTANTS_CONFIG_H 