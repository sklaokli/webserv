/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 13:45:03 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 16:39:24 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Logger.hpp"
#include <ctime>
#include <iostream>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

LogLevel Logger::_currentLevel = INFO;

void Logger::setLogLevel(LogLevel level) {
	_currentLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
	if (level < _currentLevel) return;

	const char* color = WHITE;
	const char* tag = "LOG";

	switch (level) {
		case DEBUG:
			color = CYAN;
			tag = "DEBUG";
			break;
		case INFO:
			color = GREEN;
			tag = "INFO";
			break;
		case WARNING:
			color = YELLOW;
			tag = "WARN";
			break;
		case ERROR:
			color = RED;
			tag = "ERROR";
			break;
	}

	time_t now = time(NULL);
	struct tm* t = localtime(&now);
	char time[32];
	strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", t);

	std::ostream& out = (level == ERROR) ? std::cerr : std::cout;

	out << "[" << time << "] " << color << "[" << tag << "]" << RESET << " "
	    << message << std::endl;
}

void Logger::debug(const std::string& message) {
	log(DEBUG, message);
}

void Logger::info(const std::string& message) {
	log(INFO, message);
}

void Logger::warning(const std::string& message) {
	log(WARNING, message);
}

void Logger::error(const std::string& message) {
	log(ERROR, message);
}

Logger::Logger() {}

Logger::Logger(const Logger&) {}

Logger& Logger::operator=(const Logger&) {
	return *this;
}

Logger::~Logger() {}
