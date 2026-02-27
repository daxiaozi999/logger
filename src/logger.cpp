#include "logger.h"

namespace fog {

    std::unique_ptr<Logger> Logger::instance_ = nullptr;
    std::once_flag Logger::flag_;

    Logger::Logger()
        : isFile_(true)
        , isConsole_(true)
        , minLevel_(LogLevel::Debug)
        , dropCount_(0)
        , maxQueueSize_(1000)
        , running_(true)
        , fileAppender_(std::make_unique<FileAppender>())
        , consoleAppender_(std::make_unique<ConsoleAppender>()) {
        fileThread_ = std::thread(&Logger::writeToFileLoop, this);
        flushThread_ = std::thread(&Logger::flushLoop, this);
    }

    Logger::~Logger() {
        shutdown();
    }

    Logger& Logger::getInstance() {
        std::call_once(flag_, []() {
            instance_ = std::unique_ptr<Logger>(new Logger());
            });
        return *instance_;
    }

    void Logger::writeLog(const LogEntry& entry) {
        if (!running_.load() || !shouldLog(entry.level)) return;

        if (isConsole_.load()) {
            writeToConsole(entry);
        }

        if (isFile_.load()) {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                if (entries_.size() >= maxQueueSize_.load()) {
                    dropCount_.fetch_add(1);
                    return;
                }
                entries_.push_back(entry);
            }
            fileCv_.notify_one();
        }
    }

    void Logger::writeLog(LogEntry&& entry) {
        if (!running_.load() || !shouldLog(entry.level)) return;

        if (isConsole_.load()) {
            writeToConsole(entry);
        }

        if (isFile_.load()) {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                if (entries_.size() >= maxQueueSize_.load()) {
                    dropCount_.fetch_add(1);
                    return;
                }
                entries_.push_back(std::move(entry));
            }
            fileCv_.notify_one();
        }
    }

    void Logger::flushLoop() {
        constexpr auto FLUSH_INTERVAL = std::chrono::milliseconds(1000);

        while (running_.load()) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            flushCv_.wait_for(lock, FLUSH_INTERVAL, [this] {
                return !running_.load();
                });

            lock.unlock();

            if (running_.load()) {
                if (isFile_.load() && fileAppender_) {
                    fileAppender_->flush();
                }
                if (isConsole_.load() && consoleAppender_) {
                    consoleAppender_->flush();
                }
            }
        }
    }

    void Logger::writeToFileLoop() {
        while (running_.load()) {
            std::deque<LogEntry> tempEntries;

            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                fileCv_.wait(lock, [this] {
                    return !running_.load() || !entries_.empty();
                    });

                if (!entries_.empty()) {
                    tempEntries.swap(entries_);
                }
            }

            if (!tempEntries.empty() && isFile_.load() && fileAppender_) {
                for (const auto& entry : tempEntries) {
                    fileAppender_->append(entry);
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!entries_.empty() && isFile_.load() && fileAppender_) {
                for (const auto& entry : entries_) {
                    fileAppender_->append(entry);
                }
                entries_.clear();
            }
        }
    }

    void Logger::writeToConsole(const LogEntry& entry) {
        if (consoleAppender_) {
            consoleAppender_->append(entry);
            if (entry.level == LogLevel::Error || entry.level == LogLevel::Warning) {
                consoleAppender_->flush();
            }
        }
    }

    void Logger::flush() {
        if (!running_.load()) return;

        if (isFile_.load() && fileAppender_) {
            fileAppender_->flush();
        }
        if (isConsole_.load() && consoleAppender_) {
            consoleAppender_->flush();
        }
    }

    void Logger::shutdown() {
        if (!running_.load()) return;

        running_.store(false);
        fileCv_.notify_all();
        flushCv_.notify_all();

        if (fileThread_.joinable()) {
            fileThread_.join();
        }
        if (flushThread_.joinable()) {
            flushThread_.join();
        }

        if (fileAppender_) {
            fileAppender_->flush();
        }
        if (consoleAppender_) {
            consoleAppender_->flush();
        }
    }

    void Logger::setStyle(int style) {
        LogFormat::setStyle(style);
    }

    void Logger::setStyle(LogStyle style) {
        LogFormat::setStyle(style);
    }

    void Logger::setColorEnable(bool enable) {
        LogFormat::setColorEnable(enable);
    }

    void Logger::setMaxFileSize(size_t maxsize) {
        if (fileAppender_) {
            fileAppender_->setMaxFileSize(maxsize);
        }
    }

    void Logger::setDirectory(const std::string& directory) {
        if (fileAppender_) {
            fileAppender_->setDirectory(directory);
        }
    }

    void Logger::setFileRotate(LogLevel level, bool enable) {
        if (fileAppender_) {
            fileAppender_->setFileRotate(level, enable);
        }
    }

    void Logger::setLevelFile(LogLevel level, const std::string& filename) {
        if (fileAppender_) {
            fileAppender_->setLevelFile(level, filename);
        }
    }

    void Logger::setMinLevel(int level) {
        minLevel_.store(LOG_IntToLevel(level));
    }

    void Logger::setMinLevel(LogLevel level) {
        minLevel_.store(level);
    }

    void Logger::setMaxQueueSize(size_t maxsize) {
        if (maxsize > 0) {
            maxQueueSize_.store(maxsize);
        }
    }

    void Logger::setFileEnable(bool enable) {
        isFile_.store(enable);
        if (!enable) {
            fileCv_.notify_all();
        }
    }

    void Logger::setConsoleEnable(bool enable) {
        isConsole_.store(enable);
    }

    size_t Logger::getDropCount() const {
        return dropCount_.load();
    }

    size_t Logger::getQueueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return entries_.size();
    }

    bool Logger::shouldLog(LogLevel level) const {
        return LOG_LevelEnable(level, minLevel_.load());
    }

} // namespace fog