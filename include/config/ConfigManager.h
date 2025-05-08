#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <wx/wx.h>
#include <wx/fileconf.h>
#include "logger/Logger.h"

class ConfigManager {
public:

    static ConfigManager& getInstance();

    bool initialize(const std::string& configFilePath = "");

    std::string getString(const std::string& section, const std::string& key, const std::string& defaultValue = "");

    int getInt(const std::string& section, const std::string& key, int defaultValue = 0);

    double getDouble(const std::string& section, const std::string& key, double defaultValue = 0.0);

    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false);

    void setString(const std::string& section, const std::string& key, const std::string& value);

    void setInt(const std::string& section, const std::string& key, int value);

    void setDouble(const std::string& section, const std::string& key, double value);

    void setBool(const std::string& section, const std::string& key, bool value);

    bool save();

    bool reload();

    std::string getConfigFilePath() const;

    std::vector<std::string> getSections();
    std::vector<std::string> getKeys(const std::string& section);

private:
    ConfigManager();
    ~ConfigManager();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    std::string findConfigFile();

    std::string configFilePath;
    std::unique_ptr<wxFileConfig> fileConfig;
    bool initialized;
};

#endif // CONFIG_MANAGER_H