/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:57:55 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:13:54 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>

class Utils {
public:
	template <typename T>
	static std::string toString(const T& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}
	static std::string readFile(const std::string&);
	static int toInt(const std::string& str);
	static size_t parseSize(const std::string& str);
	static bool isValidHost(const std::string& host);
private:
	Utils();
	Utils(const Utils&);
	Utils& operator=(const Utils&);
	~Utils();
};

#endif
