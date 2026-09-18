/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:59:34 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 20:40:09 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"
#include <cctype>
#include <fstream>
#include <stdexcept>

std::string Utils::readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Unable to open file: " + path);
	}
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
	if (multiplier > 1 && baseVal > (static_cast<size_t>(-1) / multiplier)) {
		throw std::runtime_error(
		    "Size value causes integer overflow: '" + str + "'");
	}
	return baseVal * multiplier;
}

bool Utils::isValidHost(const std::string& host) {
	if (host.empty()) return false;
	if (host == "localhost") return true;

	size_t start = 0;
	size_t dots = 0;

	for (size_t i = 0; i <= host.length(); ++i) {
		if (i == host.length() || host[i] == '.') {
			if (i == start || i - start > 3) return false;
			std::string octetStr = host.substr(start, i - start);
			for (size_t j = 0; j < octetStr.length(); ++j) {
				if (!std::isdigit(static_cast<unsigned char>(octetStr[j]))) {
					return false;
				}
			}
			int octet = toInt(octetStr);
			if (octet < 0 || octet > 255) return false;
			if (i < host.length()) {
				++dots;
				start = i + 1;
			}
		}
	}
	return dots == 3;
}

bool Utils::isValidMethod(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE" ||
	        method == "HEAD" || method == "PUT");
}

void Utils::assertArgs(const std::vector<Token>& tokens, size_t expected) {
	if (tokens.empty()) {
		throw std::runtime_error("Empty directive tokens");
	}
	if (tokens.size() - 1 != expected) {
		throw std::runtime_error("Directive '" + tokens[0].value +
		                         "' requires " + Utils::toString(expected) +
		                         (expected == 1 ? " argument" : " arguments") +
		                         " on line " + Utils::toString(tokens[0].line));
	}
}

void Utils::assertArgs(
    const std::vector<Token>& tokens, size_t min, size_t max) {
	if (tokens.empty()) {
		throw std::runtime_error("Empty directive tokens");
	}
	size_t count = tokens.size() - 1;
	if (count < min || count > max) {
		std::string rangeStr;
		if (min == max) {
			rangeStr =
			    Utils::toString(min) + (min == 1 ? " argument" : " arguments");
		} else if (max == min + 1) {
			rangeStr = Utils::toString(min) + " or " + Utils::toString(max) +
			           " arguments";
		} else {
			rangeStr = Utils::toString(min) + " to " + Utils::toString(max) +
			           " arguments";
		}
		throw std::runtime_error("Directive '" + tokens[0].value +
		                         "' requires " + rangeStr + " on line " +
		                         Utils::toString(tokens[0].line));
	}
}

void Utils::assertMinArgs(const std::vector<Token>& tokens, size_t min) {
	if (tokens.empty()) {
		throw std::runtime_error("Empty directive tokens");
	}
	size_t count = tokens.size() - 1;
	if (count < min) {
		throw std::runtime_error("Directive '" + tokens[0].value +
		                         "' requires at least " + Utils::toString(min) +
		                         (min == 1 ? " argument" : " arguments") +
		                         " on line " + Utils::toString(tokens[0].line));
	}
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
