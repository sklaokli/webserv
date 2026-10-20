/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:57:55 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 20:50:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <sstream>
#include <string>
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
	static std::string normalizePath(const std::string&);
	static std::string join(
	    const std::vector<std::string>&, const std::string&);
	static int toInt(const std::string&);

private:
	Utils();
	Utils(const Utils&);
	Utils& operator=(const Utils&);
	~Utils();
};

#endif
