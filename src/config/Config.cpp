/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:42:28 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:13:38 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Config.hpp"
#include "config/Parser.hpp"
#include <fstream>
#include <stdexcept>

Config::Config() : _path("") {}

Config::Config(const std::string& configPath) : _path(configPath) {}

Config::Config(const Config& other) : _path(other._path) {}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		_path = other._path;
	}
	return *this;
}

Config::~Config() {}

void Config::parse() {
	Parser parser(_path);
	*this = parser.execute();
}

const std::string& Config::getPath() const {
	return _path;
}
