#ifndef __LOG_FORMAT_H__
#define __LOG_FORMAT_H__

#include <mutex>
#include <string>
#include <sstream>
#include <iomanip>

#include "log_level.h"
#include "log_entry.h"
#include "log_utils.h"

namespace fog {

    enum class LogStyle : int {
        Style_0 = 0,	// [Level] Log
        Style_1 = 1,	// [Level] [Timestamp] Log
        Style_2 = 2,	// [Level] [Timestamp] [ThreadId] Log
        Style_3 = 3,	// [Level] [Timestamp] [ThreadId] [File:Line] Log
    };

    static inline const char* LOG_LevelToColor(LogLevel level) {
        switch (level) {
        case LogLevel::Debug:    return "\033[38;5;24m";
        case LogLevel::Info:     return "\033[38;5;22m";
        case LogLevel::Warning:  return "\033[38;5;136m";
        case LogLevel::Error:    return "\033[38;5;124m";
        case LogLevel::Off:      return "\033[38;5;240m";
        default:                 return "\033[0m";
        }
    }

    static inline const char* LOG_LevelToColor(int level) {
        return LOG_LevelToColor(LOG_IntToLevel(level));
    }

    static inline const char* LOG_GetDefaultColor() {
        return "\033[0m";
    }

    class LogFormat {
    public:
        static std::string format(const LogEntry& entry, bool isFile = false);
        static void setStyle(int style);
        static void setStyle(LogStyle style);
        static void setColorEnable(bool enable);

    private:
        static std::string format_0(const LogEntry& entry, bool isFile);
        static std::string format_1(const LogEntry& entry, bool isFile);
        static std::string format_2(const LogEntry& entry, bool isFile);
        static std::string format_3(const LogEntry& entry, bool isFile);

    private:
        static std::mutex mutex_;
        static LogStyle style_;
        static bool color_;

    private:
        LogFormat() = default;
       ~LogFormat() = default;
        LogFormat(const LogFormat&) = delete;
        LogFormat& operator=(const LogFormat&) = delete;
        LogFormat(LogFormat&&) = delete;
        LogFormat& operator=(LogFormat&&) = delete;
    };

} // namespace fog

#endif // !__LOG_FORMAT_H__