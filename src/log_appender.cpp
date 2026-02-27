#include "log_appender.h"

namespace fog {

	ConsoleAppender::ConsoleAppender() {
		std::ios::sync_with_stdio(false);
		std::cin.tie(nullptr);
		std::cout.tie(nullptr);
	}

	ConsoleAppender::~ConsoleAppender() {
		flush();
	}

	void ConsoleAppender::append(const LogEntry& entry) {
		std::lock_guard<std::mutex> lock(mutex_);
		std::string log = LogFormat::format(entry, false);
		std::cout << log << std::endl;
	}

	void ConsoleAppender::flush() {
		std::lock_guard<std::mutex> lock(mutex_);
		std::cout.flush();
	}

	FileAppender::FileAppender(const std::string& directory) : directory_(directory) {
		handleMap_[LogLevel::Debug].filename = "log.txt";
		handleMap_[LogLevel::Info].filename = "log.txt";
		handleMap_[LogLevel::Warning].filename = "log.txt";
		handleMap_[LogLevel::Error].filename = "log.txt";
		handleMap_[LogLevel::Off].filename = "log.txt";

		handleMap_[LogLevel::Debug].stream = std::make_shared<std::ofstream>();
		handleMap_[LogLevel::Info].stream = std::make_shared<std::ofstream>();
		handleMap_[LogLevel::Warning].stream = std::make_shared<std::ofstream>();
		handleMap_[LogLevel::Error].stream = std::make_shared<std::ofstream>();
		handleMap_[LogLevel::Off].stream = std::make_shared<std::ofstream>();
	}

	FileAppender::~FileAppender() {
		flush();

		std::lock_guard<std::mutex> lock(mutex_);
		for (auto& pair : handleMap_) {
			if (pair.second.stream && pair.second.stream->is_open()) {
				pair.second.stream->close();
			}
		}
	}

	void FileAppender::append(const LogEntry& entry) {
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = handleMap_.find(entry.level);
		if (it == handleMap_.end()) return;

		auto& handle = it->second;

		if (!openFile(handle)) return;
		if (handle.rotate && !rotateFile(handle)) return;
		if (handle.stream && handle.stream->is_open()) {
			std::string log = LogFormat::format(entry, true);
			*(handle.stream) << log << std::endl;
		}
	}

	void FileAppender::flush() {
		std::lock_guard<std::mutex> lock(mutex_);
		for (auto& pair : handleMap_) {
			if (pair.second.stream && pair.second.stream->is_open()) {
				pair.second.stream->flush();
			}
		}
	}

	void FileAppender::setMaxFileSize(size_t maxsize) {
		if (maxsize) return;
		std::lock_guard<std::mutex> lock(mutex_);
		for (auto& pair : handleMap_) {
			pair.second.maxsize = maxsize;
		}
	}

	void FileAppender::setDirectory(const std::string& directory) {
		std::lock_guard<std::mutex> lock(mutex_);
		directory_ = directory.empty() ? "./logs" : directory;

		for (auto& pair : handleMap_) {
			if (pair.second.stream && pair.second.stream->is_open()) {
				pair.second.stream->close();
			}
		}
	}

	void FileAppender::setFileRotate(LogLevel level, bool rotate) {
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = handleMap_.find(level);
		if (it != handleMap_.end()) {
			it->second.rotate = rotate;
		}
	}

	void FileAppender::setLevelFile(LogLevel level, const std::string& filename) {
		if (filename.empty()) return;
		std::lock_guard<std::mutex> lock(mutex_);

		auto it = handleMap_.find(level);
		if (it != handleMap_.end()) {
			if (it->second.stream && it->second.stream->is_open()) {
				it->second.stream->close();
			}
			it->second.filename = filename;
			it->second.stream = std::make_shared<std::ofstream>();
		}
	}

	bool FileAppender::openFile(FileHandle& handle) {
		if (handle.stream && handle.stream->is_open()) return true;

		if (!LOG_DirectoryExist(directory_)) {
			if (!LOG_MakeDirectoryLoop(directory_)) {
				directory_ = "./logs";
				LOG_MakeDirectory(directory_);
			}
		}

		std::string fullPath = LOG_JoinPath(directory_, handle.filename);

		if (!handle.stream) {
			handle.stream = std::make_shared<std::ofstream>();
		}

		handle.stream->open(fullPath, std::ios::app);
		return handle.stream->is_open();
	}

	bool FileAppender::rotateFile(FileHandle& handle) {
		if (!handle.stream || !handle.stream->is_open()) return false;

		std::string fullPath = LOG_JoinPath(directory_, handle.filename);

		if (LOG_RotateFile(fullPath, handle.maxsize)) {
			handle.stream->close();
			return openFile(handle);
		}

		handle.rotate = false;
		return true;
	}

} // namespace fog