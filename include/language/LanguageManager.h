#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/arrstr.h>
#include "logger/Logger.h"

class LanguageManager {
public:
    static LanguageManager& getInstance();

    bool initialize(const std::string& langFilePath = "");

    std::string getText(const std::string& key, const std::string& defaultValue = "");

    bool setLanguage(const std::string& langCode);

    std::string getCurrentLanguage() const;

    std::vector<std::pair<std::string, std::string>> getAvailableLanguages();

private:
    LanguageManager();
    ~LanguageManager();

    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;

    bool loadLanguageFile(const std::string& langCode);

    std::string findLanguageFile(const std::string& langCode);
    std::string findDefaultLanguageFile();

    std::string currentLangCode;
    std::string langFilePath;
    std::map<std::string, std::string> translations;
    bool initialized;
};

#endif // LANGUAGE_MANAGER_H