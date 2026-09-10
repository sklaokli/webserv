#include "Logger.hpp"
#include <ctime>
#include <iomanip>

LogLevel Logger::s_currentLevel = LOG_INFO;

void Logger::setLogLevel(LogLevel level) {
	s_currentLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
	if (level < s_currentLevel) return;

	time_t now = time(NULL);
	struct tm* t = localtime(&now);
	char timeBuf[32];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", t);

	const char* color = "";
	const char* reset = "\033[0m";
	const char* tag = "INFO";

	switch (level) {
		case LOG_DEBUG:
			color = "\033[36m";  // Cyan
			tag = "DEBUG";
			break;
		case LOG_INFO:
			color = "\033[32m";  // Green
			tag = "INFO";
			break;
		case LOG_WARNING:
			color = "\033[33m";  // Yellow
			tag = "WARN";
			break;
		case LOG_ERROR:
			color = "\033[31m";  // Red
			tag = "ERROR";
			break;
	}

	std::ostream& out = (level == LOG_ERROR) ? std::cerr : std::cout;
	out << "[" << timeBuf << "] " << color << "[" << tag << "] " << reset
	    << message << std::endl;
}

void Logger::debug(const std::string& message) {
	log(LOG_DEBUG, message);
}

void Logger::info(const std::string& message) {
	log(LOG_INFO, message);
}

void Logger::warning(const std::string& message) {
	log(LOG_WARNING, message);
}

void Logger::error(const std::string& message) {
	log(LOG_ERROR, message);
}
