#ifndef __LOG_ENTRY_H__
#define __LOG_ENTRY_H__

#include <string>
#include <thread>
#include <chrono>

#include "log_level.h"

namespace fog {

	struct LogEntry {

		LogLevel level;
		std::chrono::system_clock::time_point timestamp;
		std::thread::id thread_id;
		const char* filename;
		int line;
		std::string message;

		LogEntry(LogLevel lvl, const char* file, int ln, const std::string& msg)
			: level(lvl)
			, timestamp(std::chrono::system_clock::now())
			, thread_id(std::this_thread::get_id())
			, filename(file)
			, line(ln)
			, message(msg) {
		}

		LogEntry(LogLevel lvl, const char* file, int ln, std::string&& msg)
			: level(lvl)
			, timestamp(std::chrono::system_clock::now())
			, thread_id(std::this_thread::get_id())
			, filename(file)
			, line(ln)
			, message(std::move(msg)) {
		}

		LogEntry(const LogEntry&) = default;
		LogEntry& operator=(const LogEntry&) = default;
		LogEntry(LogEntry&&) = default;
		LogEntry& operator=(LogEntry&&) = default;

	};

} // namespace fog

#endif // !__LOG_ENTRY_H__