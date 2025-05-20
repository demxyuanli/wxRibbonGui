#ifndef LOGGER_H
#define LOGGER_H

#include <wx/wx.h>
#include <fstream>
#include <string>
#include <set>

class Logger {
public:
    enum class LogLevel { INF, DBG, WRN, ERR };

    static Logger& getLogger();
    void SetOutputCtrl(wxTextCtrl* ctrl);
    void Log(LogLevel level, const std::string& message, const std::string& context = "");
    void Shutdown();
    void SetLogLevels(const std::set<LogLevel>& levels, bool isSingleLevel); // Set allowed log levels
    bool ShouldLog(LogLevel level) const; // Check if a level should be logged

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile;
    wxTextCtrl* logCtrl;
    bool isShuttingDown = false;
    std::set<LogLevel> allowedLogLevels; // Set of allowed log levels
    bool isSingleLevelMode = false; // True for single-level mode (log level and above)
};

#endif