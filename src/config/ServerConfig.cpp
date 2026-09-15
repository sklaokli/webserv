/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:41:12 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include <cstddef>

ServerConfig::ServerConfig()
    : _host("127.0.0.1")
    , _port(8080)
    , _serverNames()
    , _clientMaxBodySize(1048576)
    , _root("./www")
    , _index("index.html")
    , _errorPages()
    , _locations() {}

ServerConfig::ServerConfig(const ServerConfig& other)
    : _host(other._host)
    , _port(other._port)
    , _serverNames(other._serverNames)
    , _clientMaxBodySize(other._clientMaxBodySize)
    , _root(other._root)
    , _index(other._index)
    , _errorPages(other._errorPages)
    , _locations(other._locations) {}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
	if (this != &other) {
		_host = other._host;
		_port = other._port;
		_serverNames = other._serverNames;
		_clientMaxBodySize = other._clientMaxBodySize;
		_root = other._root;
		_index = other._index;
		_errorPages = other._errorPages;
		_locations = other._locations;
	}
	return *this;
}

ServerConfig::~ServerConfig() {}

const std::string& ServerConfig::getHost() const {
	return _host;
}

int ServerConfig::getPort() const {
	return _port;
}

const std::vector<std::string>& ServerConfig::getServerNames() const {
	return _serverNames;
}

size_t ServerConfig::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

const std::string& ServerConfig::getRoot() const {
	return _root;
}

const std::string& ServerConfig::getIndex() const {
	return _index;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const {
	return _errorPages;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
	return _locations;
}

void ServerConfig::setHost(const std::string& host) {
	_host = host;
}

void ServerConfig::setPort(int port) {
	_port = port;
}

void ServerConfig::setServerNames(const std::vector<std::string>& names) {
	_serverNames = names;
}

void ServerConfig::addServerName(const std::string& name) {
	for (size_t i = 0; i < _serverNames.size(); ++i) {
		if (_serverNames[i] == name) return;
	}
	_serverNames.push_back(name);
}

void ServerConfig::setClientMaxBodySize(size_t size) {
	_clientMaxBodySize = size;
}

void ServerConfig::setRoot(const std::string& root) {
	_root = root;
}

void ServerConfig::setIndex(const std::string& index) {
	_index = index;
}

void ServerConfig::setErrorPage(int code, const std::string& uri) {
	_errorPages[code] = uri;
}

void ServerConfig::addLocation(const LocationConfig& location) {
	_locations.push_back(location);
}

void ServerConfig::finalize() {
	for (size_t i = 0; i < _locations.size(); ++i) {
		if (_locations[i].getRoot().empty()) {
			_locations[i].setRoot(_root);
		}
		if (_locations[i].getIndex().empty()) {
			_locations[i].setIndex(_index);
		}
		if (_locations[i].getClientMaxBodySize() == 0) {
			_locations[i].setClientMaxBodySize(_clientMaxBodySize);
		}
		if (_locations[i].getAllowedMethods().empty()) {
			_locations[i].addAllowedMethod("GET");
		}
	}
}

std::string ServerConfig::getErrorPage(int code) const {
	std::map<int, std::string>::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end()) {
		return it->second;
	}
	return "";
}

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
	const LocationConfig* bestMatch = NULL;
	size_t longestMatchLen = 0;

	for (size_t i = 0; i < _locations.size(); ++i) {
		const std::string& locPath = _locations[i].getPath();
		if (uri == locPath) {
			if (locPath.length() >= longestMatchLen) {
				longestMatchLen = locPath.length();
				bestMatch = &_locations[i];
			}
		} else if (locPath == "/") {
			if (1 >= longestMatchLen) {
				longestMatchLen = 1;
				bestMatch = &_locations[i];
			}
		} else if (uri.find(locPath) == 0) {
			if (locPath[locPath.length() - 1] == '/' ||
			    (uri.length() > locPath.length() &&
			        uri[locPath.length()] == '/')) {
				if (locPath.length() >= longestMatchLen) {
					longestMatchLen = locPath.length();
					bestMatch = &_locations[i];
				}
			}
		}
	}
	return bestMatch;
}
