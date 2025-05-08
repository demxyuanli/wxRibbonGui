#include "logger/Logger.h"
#include <ctime>
#include <iomanip>
#include <iostream>

Logger::Logger() : logCtrl(nullptr) {
    logFile.open("app.log", std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger& Logger::getLogger() {
    static Logger instance;
    return instance;
}

void Logger::SetOutputCtrl(wxTextCtrl* ctrl) {
    logCtrl = ctrl;
}

void Logger::Log(LogLevel level, const std::string& message, const std::string& context) {
    if (!logFile.is_open()) return;

    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    std::string levelStr;
    switch (level) {
    case LogLevel::INF: levelStr = "INF"; break;
    case LogLevel::DBG: levelStr = "DBG"; break;
    case LogLevel::WRN: levelStr = "WRN"; break;
    case LogLevel::ERR: levelStr = "ERR"; break;
    }

    std::string contextStr = context.empty() ? "" : "[" + context + "] ";

    std::string logMessage = "[" + std::string(timestamp) + "] [" + levelStr + "] " + contextStr + message;
    logFile << logMessage << std::endl;
    std::cout << "Logger: level=" << levelStr << ", message=" << logMessage << std::endl;
    logFile.flush();

    if (isShuttingDown || !logCtrl || !logCtrl->IsShown()) {
        return;
    }
    logCtrl->AppendText(logMessage + "\n");
}

void Logger::Shutdown() {
    isShuttingDown = true;
    logCtrl = nullptr;
}