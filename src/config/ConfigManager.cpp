#include "config/ConfigManager.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

ConfigManager::ConfigManager() : initialized(false) {
}

ConfigManager::~ConfigManager() {
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::initialize(const std::string& configFilePath) {
    if (initialized) {
        Logger::getLogger().Log(Logger::LogLevel::WRN, "Configuration manager already initialized", "ConfigManager");
        return true;
    }

    // If a configuration file path is provided, use it
    if (!configFilePath.empty()) {
        this->configFilePath = configFilePath;
    }
    else {
        // Otherwise try to find the configuration file
        this->configFilePath = findConfigFile();
    }

    // If no configuration file is found, create a new one
    if (this->configFilePath.empty()) {
        // Create configuration file in user configuration directory
        wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
        wxFileName exeFile(wxStandardPaths::Get().GetExecutablePath());
        wxString appName = exeFile.GetName();

        wxString configDir = userConfigDir + wxFileName::GetPathSeparator() + appName;
        if (!wxDirExists(configDir)) {
            wxMkdir(configDir);
        }

        this->configFilePath = (configDir + wxFileName::GetPathSeparator() + "config.ini").ToStdString();
    }

    // Create configuration object
    fileConfig = std::make_unique<wxFileConfig>(wxEmptyString, wxEmptyString,
        wxString(this->configFilePath), wxEmptyString,
        wxCONFIG_USE_LOCAL_FILE);

    initialized = true;
    Logger::getLogger().Log(Logger::LogLevel::INF, "Configuration manager initialized successfully, config file: " + this->configFilePath, "ConfigManager");
    return true;
}

std::string ConfigManager::getString(const std::string& section, const std::string& key, const std::string& defaultValue) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return defaultValue;
    }

    wxString value;
    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Read(wxString(key), &value, wxString(defaultValue));
    return value.ToStdString();
}

int ConfigManager::getInt(const std::string& section, const std::string& key, int defaultValue) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return defaultValue;
    }

    int value;
    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Read(wxString(key), &value, defaultValue);
    return value;
}

double ConfigManager::getDouble(const std::string& section, const std::string& key, double defaultValue) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return defaultValue;
    }

    double value;
    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Read(wxString(key), &value, defaultValue);
    return value;
}

bool ConfigManager::getBool(const std::string& section, const std::string& key, bool defaultValue) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return defaultValue;
    }

    bool value;
    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Read(wxString(key), &value, defaultValue);
    return value;
}

void ConfigManager::setString(const std::string& section, const std::string& key, const std::string& value) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return;
    }

    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Write(wxString(key), wxString(value));
}

void ConfigManager::setInt(const std::string& section, const std::string& key, int value) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return;
    }

    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Write(wxString(key), value);
}

void ConfigManager::setDouble(const std::string& section, const std::string& key, double value) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return;
    }

    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Write(wxString(key), value);
}

void ConfigManager::setBool(const std::string& section, const std::string& key, bool value) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return;
    }

    fileConfig->SetPath("/" + wxString(section));
    fileConfig->Write(wxString(key), value);
}

bool ConfigManager::save() {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return false;
    }

    return fileConfig->Flush();
}

bool ConfigManager::reload() {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return false;
    }

    // Reload configuration file
    fileConfig = std::make_unique<wxFileConfig>(wxEmptyString, wxEmptyString,
        wxString(configFilePath), wxEmptyString,
        wxCONFIG_USE_LOCAL_FILE);
    return true;
}

std::string ConfigManager::getConfigFilePath() const {
    return configFilePath;
}

std::vector<std::string> ConfigManager::getSections() {
    std::vector<std::string> sections;

    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return sections;
    }

    wxString str;
    long index;
    bool cont = fileConfig->GetFirstGroup(str, index);
    while (cont) {
        sections.push_back(str.ToStdString());
        cont = fileConfig->GetNextGroup(str, index);
    }

    return sections;
}

std::vector<std::string> ConfigManager::getKeys(const std::string& section) {
    std::vector<std::string> keys;

    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Configuration manager not initialized", "ConfigManager");
        return keys;
    }

    fileConfig->SetPath("/" + wxString(section));

    wxString str;
    long index;
    bool cont = fileConfig->GetFirstEntry(str, index);
    while (cont) {
        keys.push_back(str.ToStdString());
        cont = fileConfig->GetNextEntry(str, index);
    }

    return keys;
}

std::string ConfigManager::findConfigFile() {
    // First check current directory
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeDir(exePath);
    wxString currentDir = exeDir.GetPath();

    wxString configPath = currentDir + wxFileName::GetPathSeparator() + "config.ini";
    if (wxFileExists(configPath)) {
        return configPath.ToStdString();
    }

    // Then check user configuration directory
    wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
    wxString appName = exeDir.GetName();
    configPath = userConfigDir + wxFileName::GetPathSeparator() + appName + wxFileName::GetPathSeparator() + "config.ini";
    if (wxFileExists(configPath)) {
        return configPath.ToStdString();
    }

    return "";
}