#include "Logger.hpp"

namespace specbolt {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : log_level_(LogLevel::DEBUG), stop_logging_(false) {
    logging_thread_ = std::thread(&Logger::ProcessLogQueue, this);
}

Logger::~Logger() {
    stop_logging_ = true;
    cv_.notify_one();
    if (logging_thread_.joinable()) {
        logging_thread_.join();
    }
    if (log_file_) {
        log_file_.close();
    }
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_level_ = level;
}

void Logger::SetLogFile(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_file_.open(file_path, std::ios::out | std::ios::app);
    if (!log_file_) {
        throw std::runtime_error("Failed to open log file");
    }
}

void Logger::SetLogFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_format_ = format;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < log_level_) {
        return;
    }

    std::ostringstream log_stream;
    log_stream << FormatLogMessage(level, message);

    std::string log_message = log_stream.str();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        log_queue_.emplace(level, log_message);
    }
    cv_.notify_one();
}

std::string Logger::FormatLogMessage(LogLevel level, const std::string& message) {
    std::ostringstream ss;
    ss << GetTimestamp() << " " << GetLogLevelString(level) << " " << message;
    return ss.str();
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

std::string Logger::GetLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "[DEBUG]";
        case LogLevel::INFO: return "[INFO]";
        case LogLevel::WARNING: return "[WARNING]";
        case LogLevel::ERROR: return "[ERROR]";
        case LogLevel::CRITICAL: return "[CRITICAL]";
        default: return "[UNKNOWN]";
    }
}

std::string Logger::GetColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "\033[36m";
        case LogLevel::INFO: return "\033[32m";
        case LogLevel::WARNING: return "\033[33m";
        case LogLevel::ERROR: return "\033[31m";
        case LogLevel::CRITICAL: return "\033[41m";
        default: return "\033[0m";
    }
}

void Logger::ProcessLogQueue() {
    while (!stop_logging_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return !log_queue_.empty() || stop_logging_; });

        while (!log_queue_.empty()) {
            auto [level, message] = log_queue_.front();
            log_queue_.pop();
            lock.unlock();

            std::cout << GetColorCode(level) << message << "\033[0m" << std::endl;

            if (log_file_) {
                log_file_ << message << std::endl;
            }

            lock.lock();
        }
    }
}

}