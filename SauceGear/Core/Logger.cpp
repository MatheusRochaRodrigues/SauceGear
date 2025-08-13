#include "Logger.h"
#include <iostream>

void Logger::Init() {
    // Se quiser: abrir arquivo de log aqui
}

void Logger::Log(const std::string& message, LogLevel level) {
    switch (level) {
    case LogLevel::Info:    std::cout << "[INFO] " << message << std::endl; break;
    case LogLevel::Warning: std::cout << "[WARN] " << message << std::endl; break;
    case LogLevel::Error:   std::cerr << "[ERROR] " << message << std::endl; break;
    }
}

void Logger::Info(const std::string& message) { Log(message, LogLevel::Info); }
void Logger::Warn(const std::string& message) { Log(message, LogLevel::Warning); }
void Logger::Error(const std::string& message) { Log(message, LogLevel::Error); }
