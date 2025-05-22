#include "language/LanguageManager.h"
#include "logger/Logger.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/dir.h>

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
        LOG_WRN("Language manager already initialized", "LanguageManager");
        return true;
    }

    // Find language file based on the provided path or default location
    this->langFilePath = langFilePath.empty() ? findDefaultLanguageFile() : langFilePath;

    // Load the language file
    if (!this->langFilePath.empty()) {
        if (!wxFileExists(this->langFilePath)) {
            // If file doesn't exist, try to find it in common locations
            wxString defaultPath = findDefaultLanguageFile();
            if (defaultPath.empty() || !wxFileExists(defaultPath)) {
                LOG_ERR("Failed to load default language file", "LanguageManager");
                return false;
            }
            this->langFilePath = defaultPath.ToStdString();
        }
    }

    initialized = true;
    LOG_INF("Language manager initialized successfully, language file: " + this->langFilePath, "LanguageManager");
    return true;
}

std::string LanguageManager::getText(const std::string& key, const std::string& defaultValue) {
    if (!initialized) {
        LOG_ERR("Language manager not initialized", "LanguageManager");
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
        LOG_ERR("Language manager not initialized", "LanguageManager");
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
    if (!initialized) {
        LOG_ERR("Language manager not initialized", "LanguageManager");
        return "en";
    }

    return currentLangCode;
}

std::vector<std::pair<std::string, std::string>> LanguageManager::getAvailableLanguages() {
    std::vector<std::pair<std::string, std::string>> languages;

    if (!initialized) {
        LOG_ERR("Language manager not initialized", "LanguageManager");
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
        LOG_ERR("Language file not found for language code: " + langCode, "LanguageManager");
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

std::string LanguageManager::findDefaultLanguageFile() {
    // Look for language files in common locations

    // 1. Check application directory
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeDir(exePath);
    wxString appDir = exeDir.GetPath();
    wxString langPath = appDir + wxFileName::GetPathSeparator() + "lang";

    if (wxDirExists(langPath)) {
        wxArrayString langFiles;
        wxDir::GetAllFiles(langPath, &langFiles, "*.lang", wxDIR_FILES);
        if (!langFiles.IsEmpty()) {
            // Find language file for the current system language
            wxString sysLang = wxLocale::GetLanguageName(wxLocale::GetSystemLanguage());
            for (size_t i = 0; i < langFiles.Count(); ++i) {
                wxFileName fn(langFiles[i]);
                if (fn.GetName().Lower() == sysLang.Lower()) {
                    return langFiles[i].ToStdString();
                }
            }
            // Return first language file if no matching language file is found
            return langFiles[0].ToStdString();
        }
    }

    // 2. Check user configuration directory
    wxString userConfigDir = wxStandardPaths::Get().GetUserConfigDir();
    wxString appName = exeDir.GetName();
    langPath = userConfigDir + wxFileName::GetPathSeparator() + appName + wxFileName::GetPathSeparator() + "lang";

    if (wxDirExists(langPath)) {
        wxArrayString langFiles;
        wxDir::GetAllFiles(langPath, &langFiles, "*.lang", wxDIR_FILES);
        if (!langFiles.IsEmpty()) {
            return langFiles[0].ToStdString();
        }
    }

    // No language file found
    wxString langCode = wxLocale::GetLanguageName(wxLocale::GetSystemLanguage());
    LOG_ERR("Language file not found for language code: " + langCode.ToStdString(), "LanguageManager");
    return std::string();
}