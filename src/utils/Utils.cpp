/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:59:34 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 20:50:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

std::string Utils::readFile(const std::string& path) {
	struct stat st;
	if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		throw std::runtime_error("Path is a directory: " + path);
	}
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Unable to open file: " + path);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::string Utils::normalizePath(const std::string& path) {
	if (path.empty()) return "/";
	std::string res;
	for (size_t i = 0; i < path.length(); ++i) {
		if (path[i] == '/' && !res.empty() && res[res.length() - 1] == '/')
			continue;
		res += path[i];
	}
	if (res.length() > 1 && res[res.length() - 1] == '/')
		res.erase(res.length() - 1);
	return res;
}

std::string Utils::join(
    const std::vector<std::string>& vec, const std::string& delim) {
	std::string res;
	for (std::vector<std::string>::const_iterator it = vec.begin();
	     it != vec.end(); ++it) {
		if (it != vec.begin()) res += delim;
		res += *it;
	}
	return res;
}

int Utils::toInt(const std::string& str) {
	if (str.empty()) {
		throw std::runtime_error("Empty integer string");
	}
	for (size_t i = 0; i < str.length(); ++i) {
		if (i == 0 && (str[i] == '+' || str[i] == '-')) {
			if (str.length() == 1)
				throw std::runtime_error("Invalid integer: '" + str + "'");
			continue;
		}
		if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
			throw std::runtime_error("Invalid integer: '" + str + "'");
		}
	}
	std::istringstream iss(str);
	int value;
	if (!(iss >> value) || !iss.eof()) {
		throw std::runtime_error("Invalid integer: '" + str + "'");
	}
	return value;
}

Utils::Utils() {}

Utils::Utils(const Utils& other) {
	(void)other;
}

Utils& Utils::operator=(const Utils& other) {
	(void)other;
	return *this;
}

Utils::~Utils() {}
