#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

enum LogLevel { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR };

class Logger {
public:
	static void setLogLevel(LogLevel level);
	static void log(LogLevel level, const std::string& message);
	static void debug(const std::string& message);
	static void info(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& message);

private:
	static LogLevel s_currentLevel;
};

#endif
