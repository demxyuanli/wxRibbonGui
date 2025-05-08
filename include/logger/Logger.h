#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <wx/wx.h> 

#ifdef INF
#error "INF been defined"
#endif
#ifdef ERR
#error "ERR been defined"
#endif
#ifdef WRN
#error "WRN been defined"
#endif
#ifdef DBG
#error "DBG been defined"
#endif

class Logger {
public:
    enum class LogLevel { INF, DBG, WRN, ERR };

    static Logger& getLogger();
    void Log(LogLevel level, const std::string& message, const std::string& context = "");
    void SetOutputCtrl(wxTextCtrl* ctrl);
    void Shutdown();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool isShuttingDown;
    std::ofstream logFile;
    wxTextCtrl* logCtrl;
};

#endif // LOGGER_HPP