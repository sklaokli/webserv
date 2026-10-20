/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:42:28 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:39:31 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

const size_t Config::SizeUnits::KB;
const size_t Config::SizeUnits::MB;
const size_t Config::SizeUnits::GB;
const int Config::IpV4::MIN;
const int Config::IpV4::MAX;

Config::Config() : _path(""), _servers() {}

Config::Config(const std::string& path) : _path(path), _servers() {}

Config::Config(const Config& other)
    : _path(other._path), _servers(other._servers) {}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		_path = other._path;
		_servers = other._servers;
	}
	return *this;
}

Config::~Config() {}

const std::string& Config::getPath() const {
	return _path;
}

void Config::setPath(const std::string& path) {
	_path = path;
}

const std::vector<ServerConfig>& Config::getServers() const {
	return _servers;
}

std::vector<ServerConfig>& Config::getServers() {
	return _servers;
}

void Config::addServer(const ServerConfig& server) {
	_servers.push_back(server);
}

void Config::dump() const {
	Logger::debug("");
	Logger::debug("Total virtual servers: " + Utils::toString(_servers.size()));
	size_t i = 0;
	for (std::vector<ServerConfig>::const_iterator it = _servers.begin();
	     it != _servers.end(); ++it, ++i) {
		it->dump(i);
	}
	Logger::debug("");
}
