/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:59:34 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 04:36:36 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include <fstream>

std::string Utils::readFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) return "";
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
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
