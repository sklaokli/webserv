/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 13:45:11 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:52:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger {
public:
	enum Level { DEBUG, INFO, WARNING, ERROR };

	static void setLogLevel(Level);
	static void log(Level, const std::string&);
	static void debug(const std::string&);
	static void info(const std::string&);
	static void warning(const std::string&);
	static void error(const std::string&);

private:
	Logger();
	Logger(const Logger&);
	Logger& operator=(const Logger&);
	~Logger();

	static Level _currentLevel;
};

#endif
