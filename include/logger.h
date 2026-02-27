#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iostream>
#include <condition_variable>

#include "log_level.h"
#include "log_entry.h"
#include "log_format.h"
#include "log_appender.h"

namespace fog {

    class Logger {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;
        ~Logger();

        static Logger& getInstance();

        void writeLog(const LogEntry& entry);
        void writeLog(LogEntry&& entry);
        void flush();
        void shutdown();

        void setStyle(int style);
        void setStyle(LogStyle style);
        void setColorEnable(bool enable);

        void setMaxFileSize(size_t maxsize);
        void setDirectory(const std::string& directory);
        void setFileRotate(LogLevel level, bool rotate);
        void setLevelFile(LogLevel level, const std::string& filename);

        void setMinLevel(int level);
        void setMinLevel(LogLevel level);
        void setMaxQueueSize(size_t maxsize);

        void setFileEnable(bool enable);
        void setConsoleEnable(bool enable);

        size_t getDropCount() const;
        size_t getQueueSize() const;
        bool shouldLog(LogLevel level) const;

    private:
        Logger();

        void flushLoop();
        void writeToFileLoop();
        void writeToConsole(const LogEntry& entry);

    private:
        static std::unique_ptr<Logger> instance_;
        static std::once_flag flag_;

        std::atomic<bool> isFile_;
        std::atomic<bool> isConsole_;
        std::atomic<LogLevel> minLevel_;

        std::atomic<size_t> dropCount_;
        std::atomic<size_t> maxQueueSize_;

        std::thread fileThread_;
        std::thread flushThread_;
        std::atomic<bool> running_;

        std::deque<LogEntry> entries_;
        mutable std::mutex queueMutex_;
        std::condition_variable fileCv_;
        std::condition_variable flushCv_;

        std::unique_ptr<FileAppender> fileAppender_;
        std::unique_ptr<ConsoleAppender> consoleAppender_;
    };

} // namespace fog

namespace fog {

    class LogStream {
    public:

        LogStream(LogLevel level, const char* file, int line)
            : level_(level)
            , filename_(file)
            , line_(line)
            , moved_(false) {
        }

        ~LogStream() {
            flush();
        }

        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;

        LogStream(LogStream&& other) noexcept
            : level_(other.level_)
            , filename_(other.filename_)
            , line_(other.line_)
            , oss_(std::move(other.oss_))
            , moved_(false) {
            other.moved_ = true;
        }

        LogStream& operator=(LogStream&& other) noexcept {
            if (this != &other) {
                if (!moved_) {
                    flush();
                }

                level_ = other.level_;
                filename_ = other.filename_;
                line_ = other.line_;
                oss_ = std::move(other.oss_);
                moved_ = false;
                other.moved_ = true;
            }
            return *this;
        }

        template<typename T>
        LogStream& operator<<(const T& value) {
            if (!moved_) {
                oss_ << value;
            }
            return *this;
        }

        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (!moved_) {
                oss_ << manip;
            }
            return *this;
        }

        LogStream& operator<<(std::ios_base& (*manip)(std::ios_base&)) {
            if (!moved_) {
                oss_ << manip;
            }
            return *this;
        }

        LogStream& operator<<(std::ios& (*manip)(std::ios&)) {
            if (!moved_) {
                oss_ << manip;
            }
            return *this;
        }

        LogStream& operator<<(std::streambuf* sb) {
            if (!moved_) {
                oss_ << sb;
            }
            return *this;
        }

    private:
        void flush() {
            if (!moved_) {
                std::string msg = oss_.str();
                if (!msg.empty()) {
                    LogEntry entry(level_, filename_, line_, std::move(msg));
                    Logger::getInstance().writeLog(std::move(entry));
                }
            }
        }

    private:
        LogLevel level_;
        const char* filename_;
        int line_;
        std::ostringstream oss_;
        bool moved_;
    };

} // namespace fog

/*
    Macro Definitions - Logger System Interface

    This file contains convenience macros for the fog logging system.
    These macros provide a simplified interface to the Logger singleton
    and support various logging scenarios.
*/

// =============================================================================
// Logger Instance Operations
// =============================================================================

/** Get the singleton logger instance */
#define LOG_INSTANCE()    fog::Logger::getInstance()

/** Flush all pending log messages to their destinations */
#define LOG_FLUSH()       fog::Logger::getInstance().flush()

/** Shutdown the logger system and cleanup resources */
#define LOG_SHUTDOWN()    fog::Logger::getInstance().shutdown()

// =============================================================================
// Format Configuration
// =============================================================================

/** Set the log format style (0-3, where 3 is most detailed) */
#define LOG_SET_STYLE(style)            fog::Logger::getInstance().setStyle(style)

/** Enable or disable colored output for console logging */
#define LOG_SET_COLOR_ENABLE(enable)    fog::Logger::getInstance().setColorEnable(enable)

// =============================================================================
// File Output Configuration
// =============================================================================

/** Set maximum file size in bytes before rotation occurs */
#define LOG_SET_MAX_FILE_SIZE(maxsize)        fog::Logger::getInstance().setMaxFileSize(maxsize)

/** Set the directory path where log files will be stored */
#define LOG_SET_DIRECTORY(directory)          fog::Logger::getInstance().setDirectory(directory)

/** Enable or disable file rotation for a specific log level */
#define LOG_SET_FILE_ROTATE(level, rotate)    fog::Logger::getInstance().setFileRotate(level, rotate)

/** Set a specific filename for a particular log level */
#define LOG_SET_LEVEL_FILE(level, file)       fog::Logger::getInstance().setLevelFile(level, file)

// =============================================================================
// Logging Behavior Configuration
// =============================================================================

/** Set the minimum log level - messages below this level will be ignored */
#define LOG_SET_MIN_LEVEL(level)           fog::Logger::getInstance().setMinLevel(level)

/** Set the maximum number of messages in the internal queue before dropping */
#define LOG_SET_MAX_QUEUE_SIZE(maxsize)    fog::Logger::getInstance().setMaxQueueSize(maxsize)

// =============================================================================
// Output Destination Control
// =============================================================================

/** Enable or disable file output */
#define LOG_SET_FILE_ENABLE(enable)       fog::Logger::getInstance().setFileEnable(enable)

/** Enable or disable console output */
#define LOG_SET_CONSOLE_ENABLE(enable)    fog::Logger::getInstance().setConsoleEnable(enable)

// =============================================================================
// Status and Statistics
// =============================================================================

/** Get the number of dropped messages due to queue overflow */
#define LOG_GET_DROP_COUNT()     fog::Logger::getInstance().getDropCount()

/** Get the current number of messages waiting in the queue */
#define LOG_GET_QUEUE_SIZE()     fog::Logger::getInstance().getQueueSize()

/** Check if a specific log level is currently enabled */
#define LOG_SHOULD_LOG(level)    fog::Logger::getInstance().shouldLog(level)

// =============================================================================
// Predefined Configuration Profiles
// =============================================================================

/**
* Development configuration: Full logging with colors and console output
* - Style 3 (most detailed format)
* - Colors enabled for better readability
* - 100MB file size limit
* - Debug level and above
* - Both file and console output enabled
*/
#define LOG_INIT_DEVELOPMENT(directory) \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(true); \
        LOG_SET_MAX_FILE_SIZE(100 * 1024 * 1024); \
        LOG_SET_DIRECTORY(directory); \
        LOG_SET_MIN_LEVEL(fog::LogLevel::Debug); \
        LOG_SET_MAX_QUEUE_SIZE(1000); \
        LOG_SET_FILE_ENABLE(true); \
        LOG_SET_CONSOLE_ENABLE(true); \
    } while(0)

/**
* Production configuration: File-only logging without colors
* - Style 3 for detailed information in production logs
* - Colors disabled for cleaner file output
* - 100MB file size limit
* - Configurable minimum log level
* - File output only (no console spam)
*/
#define LOG_INIT_PRODUCTION(directory, level) \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(false); \
        LOG_SET_MAX_FILE_SIZE(100 * 1024 * 1024); \
        LOG_SET_DIRECTORY(directory); \
        LOG_SET_MIN_LEVEL(level); \
        LOG_SET_MAX_QUEUE_SIZE(1000); \
        LOG_SET_FILE_ENABLE(true); \
        LOG_SET_CONSOLE_ENABLE(false); \
    } while(0)

/**
* Testing configuration: Info level and above with colors
* - Style 3 for detailed test output
* - Colors enabled for test readability
* - 100MB file size limit
* - Info level and above (less verbose than debug)
* - Both file and console output for test visibility
*/
#define LOG_INIT_TESTING(directory) \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(true); \
        LOG_SET_MAX_FILE_SIZE(100 * 1024 * 1024); \
        LOG_SET_DIRECTORY(directory); \
        LOG_SET_MIN_LEVEL(fog::LogLevel::Info); \
        LOG_SET_MAX_QUEUE_SIZE(1000); \
        LOG_SET_FILE_ENABLE(true); \
        LOG_SET_CONSOLE_ENABLE(true); \
    } while (0)

/**
* Debug configuration: Maximum verbosity and larger file limits
* - Style 3 for maximum detail
* - Colors enabled for debug session readability
* - 200MB file size limit (larger for debug sessions)
* - Debug level and above (maximum verbosity)
* - Both file and console output
*/
#define LOG_INIT_DEBUG(directory) \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(true); \
        LOG_SET_MAX_FILE_SIZE(200 * 1024 * 1024); \
        LOG_SET_DIRECTORY(directory); \
        LOG_SET_MIN_LEVEL(fog::LogLevel::Debug); \
        LOG_SET_MAX_QUEUE_SIZE(1000); \
        LOG_SET_FILE_ENABLE(true); \
        LOG_SET_CONSOLE_ENABLE(true); \
    } while(0)

/**
* Console-only configuration: Quick setup for console output only
* - Style 3 for detailed console output
* - Colors enabled for terminal readability
* - Debug level and above
* - Console output only (no file I/O)
*/
#define LOG_INIT_CONSOLE() \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(true); \
        LOG_SET_MIN_LEVEL(fog::LogLevel::Debug); \
        LOG_SET_FILE_ENABLE(false); \
        LOG_SET_CONSOLE_ENABLE(true); \
    } while(0)

/**
* File-only configuration: Silent file logging without console output
* - Style 3 for detailed file logs
* - Colors disabled (not needed for files)
* - 100MB file size limit
* - Debug level and above
* - File output only (silent operation)
*/
#define LOG_INIT_FILE(directory) \
    do { \
        LOG_SET_STYLE(3); \
        LOG_SET_COLOR_ENABLE(false); \
        LOG_SET_MAX_FILE_SIZE(100 * 1024 * 1024); \
        LOG_SET_DIRECTORY(directory); \
        LOG_SET_MIN_LEVEL(fog::LogLevel::Debug); \
        LOG_SET_MAX_QUEUE_SIZE(1000); \
        LOG_SET_FILE_ENABLE(true); \
        LOG_SET_CONSOLE_ENABLE(false); \
    } while(0)

// =============================================================================
// Primary Logging Interface
// =============================================================================

/**
* Main logging macro - creates a LogStream for the specified level
* Usage: LOG(Debug) << "message" << variable;
*
* @param level Log level without fog::LogLevel:: prefix (Debug, Info, Warning, Error)
*/
#define LOG(level) fog::LogStream(fog::LogLevel::level, __FILE__, __LINE__)

// =============================================================================
// Convenience Logging Macros
// =============================================================================

/** Create a debug-level log stream - most verbose level */
#define LOGD()   fog::LogStream(fog::LogLevel::Debug, __FILE__, __LINE__)

/** Create an info-level log stream - general information */
#define LOGI()   fog::LogStream(fog::LogLevel::Info, __FILE__, __LINE__)

/** Create a warning-level log stream - potential issues */
#define LOGW()   fog::LogStream(fog::LogLevel::Warning, __FILE__, __LINE__)

/** Create an error-level log stream - actual problems */
#define LOGE()   fog::LogStream(fog::LogLevel::Error, __FILE__, __LINE__)

// =============================================================================
// Conditional Logging Macros
// =============================================================================

/**
* Log only if condition is true - prevents unnecessary string operations
* Usage: LOG_IF(Debug, x > 0) << "x is positive: " << x;
*
* @param level Log level without fog::LogLevel:: prefix
* @param condition Boolean expression that must be true to log
*/
#define LOG_IF(level, condition) \
    if (condition) LOG(level)

 /** Log debug message only if condition is true */
#define LOGD_IF(condition)   LOG_IF(Debug, condition)

/** Log info message only if condition is true */
#define LOGI_IF(condition)   LOG_IF(Info, condition)

/** Log warning message only if condition is true */
#define LOGW_IF(condition)   LOG_IF(Warning, condition)

/** Log error message only if condition is true */
#define LOGE_IF(condition)   LOG_IF(Error, condition)

#endif // !__LOGGER_H__