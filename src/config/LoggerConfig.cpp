#include "config/LoggerConfig.h"
#include "config/ConfigManager.h"
#include "logger/Logger.h"
#include <sstream>
#include <algorithm>

LoggerConfig& LoggerConfig::getInstance() {
    static LoggerConfig instance;
    return instance;
}

void LoggerConfig::initialize(ConfigManager& configManager) {
    std::string logLevelStr = configManager.getString("Logger", "LogLevel", "WRN");
    Logger::getLogger().Log(Logger::LogLevel::INF, "Reading LogLevel from config: " + logLevelStr, "LoggerConfig");
    configureLoggerLevels(logLevelStr);
    Logger::getLogger().Log(Logger::LogLevel::INF, "Logger levels set to: " + logLevelStr, "LoggerConfig");
}

void LoggerConfig::configureLoggerLevels(const std::string& logLevelStr) {
    std::set<Logger::LogLevel> logLevels;
    bool isSingleLevel = true;

    logLevels.insert(Logger::LogLevel::ERR);

    // Parse comma-separated log levels
    std::stringstream ss(logLevelStr);
    std::string level;
    while (std::getline(ss, level, ',')) {
        // Trim whitespace
        level.erase(0, level.find_first_not_of(" \t"));
        level.erase(level.find_last_not_of(" \t") + 1);
        // Convert to uppercase for case-insensitive parsing
        std::transform(level.begin(), level.end(), level.begin(), ::toupper);

        if (level == "WRN") {
            logLevels.insert(Logger::LogLevel::WRN);
            logLevels.insert(Logger::LogLevel::DBG);
            logLevels.insert(Logger::LogLevel::INF);
        }
        else if (level == "DBG") {
            logLevels.insert(Logger::LogLevel::DBG);
            logLevels.insert(Logger::LogLevel::INF);
        }
        else if (level == "INF") {
            logLevels.insert(Logger::LogLevel::INF);
        }
        else if (level != "ERR") { 
            Logger::getLogger().Log(Logger::LogLevel::WRN, "Unknown log level in config: " + level, "LoggerConfig");
        }
    }

    if (logLevels.size() == 1) { 
        logLevels.insert(Logger::LogLevel::WRN);
        logLevels.insert(Logger::LogLevel::DBG);
        logLevels.insert(Logger::LogLevel::INF);
        Logger::getLogger().Log(Logger::LogLevel::INF, "Using default log level: WRN", "LoggerConfig");
    }

    isSingleLevel = (logLevelStr.find(',') == std::string::npos);
    Logger::getLogger().Log(Logger::LogLevel::INF, "Parsed log levels count: " + std::to_string(logLevels.size()) + ", isSingleLevel: " + (isSingleLevel ? "true" : "false"), "LoggerConfig");

    // Log parsed levels
    std::string levelsStr;
    if (logLevels.count(Logger::LogLevel::ERR)) levelsStr += "ERR ";
    if (logLevels.count(Logger::LogLevel::WRN)) levelsStr += "WRN ";
    if (logLevels.count(Logger::LogLevel::DBG)) levelsStr += "DBG ";
    if (logLevels.count(Logger::LogLevel::INF)) levelsStr += "INF ";
    
    Logger::getLogger().Log(Logger::LogLevel::INF, "Active log levels: " + (levelsStr.empty() ? "none" : levelsStr), "LoggerConfig");

    // Set log levels in Logger
    Logger::getLogger().SetLogLevels(logLevels, isSingleLevel);
}