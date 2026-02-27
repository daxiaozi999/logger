#ifndef __LOG_LEVEL_H__
#define __LOG_LEVEL_H__

namespace fog {

    enum class LogLevel : int {
        Debug   = 0,
        Info    = 1,
        Warning = 2,
        Error   = 3,
        Off     = 4
    };

    static inline const char* LOG_LevelToString(LogLevel level) {
        switch (level) {
        case LogLevel::Debug:   return "D";
        case LogLevel::Info:    return "I";
        case LogLevel::Warning: return "W";
        case LogLevel::Error:   return "E";
        case LogLevel::Off:     return "O";
        default:                return "?";
        }
    }

    static inline LogLevel LOG_IntToLevel(int level) {
        if (level < 0) return LogLevel::Debug;
        if (level > static_cast<int>(LogLevel::Off)) return LogLevel::Off;
        return static_cast<LogLevel>(level);
    }

    static inline const char* LOG_LevelToString(int level) {
        return LOG_LevelToString(LOG_IntToLevel(level));
    }

    static inline bool LOG_LevelEnable(LogLevel current, LogLevel threshold) {
        return static_cast<int>(current) >= static_cast<int>(threshold);
    }

} // namespace fog

#endif // !__LOG_LEVEL_H__