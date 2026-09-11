/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 13:45:11 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/11 14:18:25 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

enum LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
	Logger();
	~Logger();

	static void setLogLevel(LogLevel level);
	static void log(LogLevel level, const std::string& message);
	static void debug(const std::string& message);
	static void info(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& message);
private:
	static LogLevel _currentLevel;
};

#endif
