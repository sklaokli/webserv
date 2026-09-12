/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 13:45:11 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/12 17:09:12 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

enum LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
	static void setLogLevel(LogLevel);
	static void log(LogLevel, const std::string&);
	static void debug(const std::string&);
	static void info(const std::string&);
	static void warning(const std::string&);
	static void error(const std::string&);
private:
	Logger();
	Logger(const Logger&);
	Logger& operator=(const Logger&);
	~Logger();

	static LogLevel _currentLevel;
};

#endif
