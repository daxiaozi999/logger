#ifndef __LOG_APPENDER_H__
#define __LOG_APPENDER_H__

#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>

#include "log_level.h"
#include "log_entry.h"
#include "log_utils.h"
#include "log_format.h"

namespace fog {

	class LogAppender {
	public:
		virtual ~LogAppender() = default;
		virtual void append(const LogEntry& entry) = 0;
		virtual void flush() = 0;
	};

	class ConsoleAppender : public LogAppender {
	public:
		ConsoleAppender();
	   ~ConsoleAppender();
		ConsoleAppender(const ConsoleAppender&) = delete;
		ConsoleAppender& operator=(const ConsoleAppender&) = delete;
		ConsoleAppender(ConsoleAppender&&) = default;
		ConsoleAppender& operator=(ConsoleAppender&&) = default;

		void append(const LogEntry& entry) override;
		void flush() override;

	private:
		std::mutex mutex_;
	};

	class FileAppender : public LogAppender {
	public:
		explicit FileAppender(const std::string& directory = "./logs");
	   ~FileAppender();
		FileAppender(const FileAppender&) = delete;
		FileAppender& operator=(const FileAppender&) = delete;
		FileAppender(FileAppender&&) = default;
		FileAppender& operator=(FileAppender&&) = default;

		struct FileHandle {
			size_t maxsize = 100 * 1024 * 1024;
			bool rotate = true;
			std::string filename = "log.txt";
			std::shared_ptr<std::ofstream> stream = nullptr;
		};

		void append(const LogEntry& entry) override;
		void flush() override;
		void setMaxFileSize(size_t maxsize);
		void setDirectory(const std::string& directory);
		void setFileRotate(LogLevel level, bool rotate);
		void setLevelFile(LogLevel level, const std::string& filename);

	private:
		bool openFile(FileHandle& handle);
		bool rotateFile(FileHandle& handle);

	private:
		std::mutex mutex_;
		std::string directory_;
		std::map<LogLevel, FileHandle> handleMap_;
	};

} // namespace fog

#endif // !__LOG_APPENDER_H__