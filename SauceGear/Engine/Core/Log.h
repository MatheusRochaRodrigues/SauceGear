#pragma once

#include <string>
#include <iostream> 

#define LOG_INFO(fmt, ...)  std::cout << "[INFO] "  << fmt << std::endl
#define LOG_WARN(fmt, ...)  std::cout << "[WARN] "  << fmt << std::endl
#define LOG_ERROR(fmt, ...) std::cout << "[ERROR] " << fmt << std::endl

//#ifdef _DEBUG
#define ASSERT(x) \
        if (!(x)) { \
            LOG_ERROR("ASSERT FAILED: {} ({}:{})", #x, __FILE__, __LINE__); \
            std::abort(); \
        }
//#else
//#define ASSERT(x) ((void)0)
//#endif


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
 
