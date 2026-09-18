/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:57:55 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 20:56:23 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "parser/Lexer.hpp"
#include <cstddef>
#include <sstream>
#include <vector>

class Utils {
public:
	template <typename T>
	static std::string toString(const T& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}
	static std::string readFile(const std::string&);
	static int toInt(const std::string&);
	static size_t parseSize(const std::string&);
	static bool isValidHost(const std::string&);
	static bool isValidPort(int);
	static bool isValidMethod(const std::string&);
	static bool isValidOnOff(const std::string&);
	static bool isValidErrorCode(int);
	static bool isValidRedirectCode(int);
	static bool isValidLocationPath(const std::string&);

	static void assertArgs(const std::vector<Token>&, size_t);
	static void assertArgs(const std::vector<Token>&, size_t, size_t);
	static void assertMinArgs(const std::vector<Token>&, size_t);

private:
	Utils();
	Utils(const Utils&);
	Utils& operator=(const Utils&);
	~Utils();
};

#endif
