/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 13:45:03 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/11 14:52:34 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <ctime>
#include <iostream>

LogLevel Logger::_currentLevel = INFO;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::setLogLevel(LogLevel level) {
	_currentLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
	if (level < _currentLevel) {
		return;
	}

	time_t now = time(NULL);
	struct tm* t = localtime(&now);
	char time[32];
	strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", t);

	const char* color = "";
	const char* reset = "\033[0m";
	const char* tag = "INFO";

	switch (level) {
		case DEBUG:
			color = "\033[36m";  // Cyan
			tag = "DEBUG";
			break;
		case INFO:
			color = "\033[32m";  // Green
			tag = "INFO";
			break;
		case WARNING:
			color = "\033[33m";  // Yellow
			tag = "WARN";
			break;
		case ERROR:
			color = "\033[31m";  // Red
			tag = "ERROR";
			break;
	}

	std::ostream& out = (level == ERROR) ? std::cerr : std::cout;
	out << "[" << time << "] " << color << "[" << tag << "] " << reset
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
