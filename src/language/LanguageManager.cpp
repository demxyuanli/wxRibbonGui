#include "language/LanguageManager.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/fileconf.h>

LanguageManager::LanguageManager() : initialized(false) {
}

LanguageManager::~LanguageManager() {
}

LanguageManager& LanguageManager::getInstance() {
    static LanguageManager instance;
    return instance;
}

bool LanguageManager::initialize(const std::string& langFilePath) {
    if (initialized) {
        Logger::getLogger().Log(Logger::LogLevel::WRN, "Language manager already initialized", "LanguageManager");
        return true;
    }

    // If a language file path is provided, use it
    if (!langFilePath.empty()) {
        this->langFilePath = langFilePath;
    }
    else {
        // Otherwise try to find the language file for the default language
        this->langFilePath = findLanguageFile("en");
    }

    // If no language file is found, create a default one
    if (this->langFilePath.empty()) {
        // Create language file in user configuration directory
        wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
        wxFileName exeFile(wxStandardPaths::Get().GetExecutablePath());
        wxString appName = exeFile.GetName();

        wxString langDir = userConfigDir + wxFileName::GetPathSeparator() + appName + wxFileName::GetPathSeparator() + "languages";
        if (!wxDirExists(langDir)) {
            wxMkdir(langDir);
        }

        this->langFilePath = (langDir + wxFileName::GetPathSeparator() + "en.ini").ToStdString();

        // Create a default language file
        wxFileConfig langConfig(wxEmptyString, wxEmptyString, this->langFilePath, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
        langConfig.SetPath("/General");
        langConfig.Write("LanguageName", "English");
        langConfig.Flush();
    }

    // Load the default language
    if (!loadLanguageFile("en")) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Failed to load default language file", "LanguageManager");
        return false;
    }

    initialized = true;
    Logger::getLogger().Log(Logger::LogLevel::INF, "Language manager initialized successfully, language file: " + this->langFilePath, "LanguageManager");
    return true;
}

std::string LanguageManager::getText(const std::string& key, const std::string& defaultValue) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Language manager not initialized", "LanguageManager");
        return defaultValue;
    }

    auto it = translations.find(key);
    if (it != translations.end()) {
        return it->second;
    }

    return defaultValue;
}

bool LanguageManager::setLanguage(const std::string& langCode) {
    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Language manager not initialized", "LanguageManager");
        return false;
    }

    if (langCode == currentLangCode) {
        return true;
    }

    if (loadLanguageFile(langCode)) {
        currentLangCode = langCode;
        return true;
    }

    return false;
}

std::string LanguageManager::getCurrentLanguage() const {
    return currentLangCode;
}

std::vector<std::pair<std::string, std::string>> LanguageManager::getAvailableLanguages() {
    std::vector<std::pair<std::string, std::string>> languages;

    if (!initialized) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Language manager not initialized", "LanguageManager");
        return languages;
    }

    // Get the language directory
    wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
    wxFileName exeFile(wxStandardPaths::Get().GetExecutablePath());
    wxString appName = exeFile.GetName();
    wxString langDir = userConfigDir + wxFileName::GetPathSeparator() + appName + wxFileName::GetPathSeparator() + "languages";

    if (!wxDirExists(langDir)) {
        return languages;
    }

    // Get all .ini files in the directory
    wxArrayString files;
    wxDir::GetAllFiles(langDir, &files, "*.ini", wxDIR_FILES);

    for (size_t i = 0; i < files.GetCount(); i++) {
        wxString fullPath = files[i];
        wxFileName fileName(fullPath);

        // Try to read language information
        wxFileConfig config(wxEmptyString, wxEmptyString, fullPath, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);

        wxString langCode = fileName.GetName(); // Gets filename without extension

        wxString langName;
        config.SetPath("/General");
        config.Read("LanguageName", &langName, langCode);

        languages.push_back(std::make_pair(langCode.ToStdString(), langName.ToStdString()));
    }

    return languages;
}

bool LanguageManager::loadLanguageFile(const std::string& langCode) {
    std::string filePath = findLanguageFile(langCode);
    if (filePath.empty()) {
        Logger::getLogger().Log(Logger::LogLevel::ERR, "Language file not found for language code: " + langCode, "LanguageManager");
        return false;
    }

    translations.clear();

    wxFileConfig langConfig(wxEmptyString, wxEmptyString, filePath, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);

    // Read all sections and keys
    wxString str;
    long index;
    bool cont = langConfig.GetFirstGroup(str, index);
    while (cont) {
        wxString section = str;
        langConfig.SetPath("/" + section);

        wxString key;
        long keyIndex;
        bool keyCont = langConfig.GetFirstEntry(key, keyIndex);
        while (keyCont) {
            wxString value;
            langConfig.Read(key, &value);

            // Store the translation with section prefix
            translations[section.ToStdString() + "." + key.ToStdString()] = value.ToStdString();

            keyCont = langConfig.GetNextEntry(key, keyIndex);
        }

        langConfig.SetPath("/");
        cont = langConfig.GetNextGroup(str, index);
    }

    return true;
}

std::string LanguageManager::findLanguageFile(const std::string& langCode) {
    // First check current directory
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeDir(exePath);
    wxString currentDir = exeDir.GetPath();

    wxString langPath = currentDir + wxFileName::GetPathSeparator() + "languages" + wxFileName::GetPathSeparator() + langCode + ".ini";
    if (wxFileExists(langPath)) {
        return langPath.ToStdString();
    }

    // Then check user configuration directory
    wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
    wxString appName = exeDir.GetName();
    langPath = userConfigDir + wxFileName::GetPathSeparator() + appName + wxFileName::GetPathSeparator() + "languages" + wxFileName::GetPathSeparator() + langCode + ".ini";
    if (wxFileExists(langPath)) {
        return langPath.ToStdString();
    }

    return "";
}