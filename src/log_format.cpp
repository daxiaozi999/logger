#include "log_format.h"

namespace fog {

    std::mutex LogFormat::mutex_;
    LogStyle LogFormat::style_ = LogStyle::Style_3;
    bool LogFormat::color_ = true;

    std::string LogFormat::format(const LogEntry& entry, bool isFile) {
        LogStyle currentStyle;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentStyle = style_;
        }

        switch (currentStyle) {
        case LogStyle::Style_0: return format_0(entry, isFile);
        case LogStyle::Style_1: return format_1(entry, isFile);
        case LogStyle::Style_2: return format_2(entry, isFile);
        case LogStyle::Style_3: return format_3(entry, isFile);
        default:                return format_3(entry, isFile);
        }
    }

    void LogFormat::setStyle(int style) {
        std::lock_guard<std::mutex> lock(mutex_);
        style_ = static_cast<LogStyle>(style < 0 ? 0 : style > 3 ? 3 : style);
    }

    void LogFormat::setStyle(LogStyle style) {
        std::lock_guard<std::mutex> lock(mutex_);
        style_ = style;
    }

    void LogFormat::setColorEnable(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        color_ = enable;
    }

    std::string LogFormat::format_0(const LogEntry& entry, bool isFile) {
        std::ostringstream oss;

        bool useColor;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            useColor = color_ && !isFile;
        }

        if (useColor) {
            oss << LOG_LevelToColor(entry.level);
        }

        // [Level] Message
        oss << "[" << LOG_LevelToString(entry.level) << "] "
            << entry.message;

        if (useColor) {
            oss << LOG_GetDefaultColor();
        }

        return oss.str();
    }

    std::string LogFormat::format_1(const LogEntry& entry, bool isFile) {
        std::ostringstream oss;

        bool useColor;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            useColor = color_ && !isFile;
        }

        if (useColor) {
            oss << LOG_LevelToColor(entry.level);
        }

        // [Level] [Timestamp] Message
        oss << "[" << LOG_LevelToString(entry.level) << "] "
            << "[" << LOG_ChronoToString(entry.timestamp) << "] "
            << entry.message;

        if (useColor) {
            oss << LOG_GetDefaultColor();
        }

        return oss.str();
    }

    std::string LogFormat::format_2(const LogEntry& entry, bool isFile) {
        std::ostringstream oss;

        bool useColor;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            useColor = color_ && !isFile;
        }

        if (useColor) {
            oss << LOG_LevelToColor(entry.level);
        }

        // [Level] [Timestamp] [ThreadId] Message
        oss << "[" << LOG_LevelToString(entry.level) << "] "
            << "[" << LOG_ChronoToString(entry.timestamp) << "] "
            << "[" << LOG_ThreadIdToString(entry.thread_id) << "] "
            << entry.message;

        if (useColor) {
            oss << LOG_GetDefaultColor();
        }

        return oss.str();
    }

    std::string LogFormat::format_3(const LogEntry& entry, bool isFile) {
        std::ostringstream oss;

        bool useColor;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            useColor = color_ && !isFile;
        }

        if (useColor) {
            oss << LOG_LevelToColor(entry.level);
        }

        // [Level] [Timestamp] [ThreadId] [File:Line] Message
        oss << "[" << LOG_LevelToString(entry.level) << "] "
            << "[" << LOG_ChronoToString(entry.timestamp) << "] "
            << "[" << LOG_ThreadIdToString(entry.thread_id) << "] "
            << "[" << LOG_ExtractFileName(entry.filename ? entry.filename : "unknown") << ":" << entry.line << "] "
            << entry.message;

        if (useColor) {
            oss << LOG_GetDefaultColor();
        }

        return oss.str();
    }

} // namespace fog