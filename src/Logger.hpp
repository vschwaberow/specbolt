#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace specbolt {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& Instance();

    void SetLogLevel(LogLevel level);
    void SetLogFile(const std::string& file_path);
    void SetLogFormat(const std::string& format);
    void Log(LogLevel level, const std::string& message, const std::string& class_name = "", const std::string& method_name = "");

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string FormatLogMessage(LogLevel level, const std::string& message, const std::string& class_name, const std::string& method_name);
    std::string GetTimestamp();
    std::string GetLogLevelString(LogLevel level);
    std::string GetColorCode(LogLevel level);
    void ProcessLogQueue();

    LogLevel log_level_;
    std::ofstream log_file_;
    std::string log_format_;
    std::mutex mutex_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::queue<std::pair<LogLevel, std::string>> log_queue_;
    std::thread logging_thread_;
    std::atomic<bool> stop_logging_;
};

}