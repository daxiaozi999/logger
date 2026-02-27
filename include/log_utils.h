#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__

#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdio>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace fog {

	static inline std::string LOG_ChronoToString(const std::chrono::system_clock::time_point& timestamp) {
		auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) % 1000;
		std::ostringstream oss;

#ifdef _WIN32
		struct tm tm_buf = {};
		if (localtime_s(&tm_buf, &time_t_val) == 0) {
			oss << std::put_time(&tm_buf, "%H:%M:%S");
		}
		else {
			oss << "??:??:??";
		}
#else
		struct tm tm_buf = {};
		if (localtime_r(&time_t_val, &tm_buf) != nullptr) {
			oss << std::put_time(&tm_buf, "%H:%M:%S");
		}
		else {
			oss << "??:??:??";
		}
#endif

		oss << "." << std::setfill('0') << std::setw(3) << ms.count();
		return oss.str();
	}

	static inline std::string LOG_ChronoToStringForFile(const std::chrono::system_clock::time_point& timestamp) {
		auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) % 1000;
		std::ostringstream oss;

#ifdef _WIN32
		struct tm tm_buf = {};
		if (localtime_s(&tm_buf, &time_t_val) == 0) {
			oss << std::put_time(&tm_buf, "%Y-%m-%d_%H-%M-%S");
		}
		else {
			oss << static_cast<long long>(time_t_val);
		}
#else
		struct tm tm_buf = {};
		if (localtime_r(&time_t_val, &tm_buf) != nullptr) {
			oss << std::put_time(&tm_buf, "%Y-%m-%d_%H-%M-%S");
		}
		else {
			oss << static_cast<long long>(time_t_val);
		}
#endif

		oss << "-" << std::setfill('0') << std::setw(3) << ms.count();
		return oss.str();
	}

	static inline std::string LOG_ThreadIdToString(const std::thread::id& thread_id) {
		std::ostringstream oss;
		oss << thread_id;
		return oss.str();
	}

	static inline std::string LOG_ExtractFileName(const std::string& filepath) {
		if (filepath.empty()) return "unknown";

		size_t pos = filepath.find_last_of("/\\");
		if (pos != std::string::npos) {
			return filepath.substr(pos + 1);
		}

		return filepath;
	}

	static inline bool LOG_DirectoryExist(const std::string& directory) {
		if (directory.empty()) return false;

#ifdef _WIN32
		struct _stat info;
		if (_stat(directory.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & _S_IFDIR) != 0;
#else
		struct stat info;
		if (stat(directory.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & S_IFDIR) != 0;
#endif
	}

	static inline std::string LOG_NormalizePath(const std::string& path) {
		std::string normalized = path;

		while (normalized.length() > 1 && (normalized.back() == '/' || normalized.back() == '\\')) {
			normalized.pop_back();
		}

#ifdef _WIN32
		for (size_t i = 0; i < normalized.length(); ++i) {
			if (normalized[i] == '/') {
				normalized[i] = '\\';
			}
		}
#else
		for (size_t i = 0; i < normalized.length(); ++i) {
			if (normalized[i] == '\\') {
				normalized[i] = '/';
			}
		}
#endif

		return normalized;
	}

	static inline bool LOG_MakeDirectory(const std::string& directory) {
		if (directory.empty()) return false;
		if (LOG_DirectoryExist(directory)) return true;

#ifdef _WIN32
		return _mkdir(directory.c_str()) == 0;
#else
		return mkdir(directory.c_str(), 0755) == 0;
#endif
	}

	static inline bool LOG_MakeDirectoryLoop(const std::string& directory) {
		if (directory.empty()) return false;
		if (LOG_DirectoryExist(directory)) return true;

		std::string current;
		std::string normalized = LOG_NormalizePath(directory);

#ifdef _WIN32
		const char separator = '\\';
#else
		const char separator = '/';
#endif

		for (size_t i = 0; i < normalized.length(); ++i) {
			current += normalized[i];

			if (normalized[i] == separator || i == normalized.length() - 1) {
				if (!current.empty() && current.length() > 1) {
					if (!LOG_DirectoryExist(current)) {
						if (!LOG_MakeDirectory(current)) {
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	static inline std::string LOG_JoinPath(const std::string& path1, const std::string& path2) {
		if (path1.empty()) return LOG_NormalizePath(path2);
		if (path2.empty()) return LOG_NormalizePath(path1);

		std::string result = path1;

#ifdef _WIN32
		const char separator = '\\';
#else
		const char separator = '/';
#endif

		if (result.back() != separator) {
			result += separator;
		}

		std::string p2 = path2;
		while (!p2.empty() && (p2.front() == '/' || p2.front() == '\\')) {
			p2 = p2.substr(1);
		}

		result += p2;
		return LOG_NormalizePath(result);
	}

	static inline bool LOG_IsFile(const std::string& file) {
		if (file.empty()) return false;

#ifdef _WIN32
		struct _stat info;
		if (_stat(file.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & _S_IFREG) != 0;
#else
		struct stat info;
		if (stat(file.c_str(), &info) != 0) {
			return false;
		}
		return (info.st_mode & S_IFREG) != 0;
#endif
	}

	static inline size_t LOG_FileSize(const std::string& file) {
		if (file.empty() || !LOG_IsFile(file)) return 0;

#ifdef _WIN32
		struct _stat info;
		if (_stat(file.c_str(), &info) != 0) {
			return 0;
		}
		return static_cast<size_t>(info.st_size);
#else
		struct stat info;
		if (stat(file.c_str(), &info) != 0) {
			return 0;
		}
		return static_cast<size_t>(info.st_size);
#endif
	}

	static inline bool LOG_RotateFile(const std::string& filepath, size_t maxsize) {
		if (filepath.empty() || maxsize == 0) return false;

		if (!LOG_IsFile(filepath) || LOG_FileSize(filepath) <= maxsize) {
			return true;
		}

		std::string timestamp = LOG_ChronoToStringForFile(std::chrono::system_clock::now());
		std::string basename = filepath;
		std::string extension = "";
		size_t dotpos = filepath.find_last_of('.');
		if (dotpos != std::string::npos && dotpos > filepath.find_last_of("/\\")) {
			basename = filepath.substr(0, dotpos);
			extension = filepath.substr(dotpos);
		}

		std::string backuppath = basename + "_" + timestamp + extension;

		return std::rename(filepath.c_str(), backuppath.c_str()) == 0;
	}

} // namespace fog

#endif // !__LOG_UTILS_H__