#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void Init(); // Pode configurar arquivo de log, etc.

    static void Log(const std::string& message, LogLevel level = LogLevel::Info);

    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);
};

#endif
