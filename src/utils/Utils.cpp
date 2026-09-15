/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:59:34 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:28:33 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"
#include <cctype>
#include <fstream>
#include <stdexcept>

std::string Utils::readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) return "";
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
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

size_t Utils::parseSize(const std::string& str) {
	if (str.empty()) {
		throw std::runtime_error("Empty size string");
	}
	char unit = str[str.length() - 1];
	size_t multiplier = 1;
	std::string numStr = str;

	if (unit == 'k' || unit == 'K') {
		multiplier = 1024;
		numStr = str.substr(0, str.length() - 1);
	} else if (unit == 'm' || unit == 'M') {
		multiplier = 1024 * 1024;
		numStr = str.substr(0, str.length() - 1);
	} else if (unit == 'g' || unit == 'G') {
		multiplier = 1024 * 1024 * 1024;
		numStr = str.substr(0, str.length() - 1);
	}

	if (numStr.empty()) {
		throw std::runtime_error("Invalid size: '" + str + "'");
	}
	for (size_t i = 0; i < numStr.length(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(numStr[i]))) {
			throw std::runtime_error(
			    "Invalid character in size: '" + str + "'");
		}
	}

	std::istringstream iss(numStr);
	size_t baseVal;
	if (!(iss >> baseVal) || !iss.eof()) {
		throw std::runtime_error("Invalid size value: '" + str + "'");
	}
	return baseVal * multiplier;
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
