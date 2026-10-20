/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:44:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <cstddef>

const int ServerConfig::Ports::MIN;
const int ServerConfig::Ports::MAX;
const int ServerConfig::StatusCodes::MIN;
const int ServerConfig::StatusCodes::MAX;

ServerConfig::ServerConfig()
    : _host("127.0.0.1")
    , _port(8080)
    , _serverNames()
    , _clientMaxBodySize(1048576)
    , _root("./www")
    , _index("index.html")
    , _errorPages()
    , _locations()
    , _isDefault(false) {}

ServerConfig::ServerConfig(const ServerConfig& other)
    : _host(other._host)
    , _port(other._port)
    , _serverNames(other._serverNames)
    , _clientMaxBodySize(other._clientMaxBodySize)
    , _root(other._root)
    , _index(other._index)
    , _errorPages(other._errorPages)
    , _locations(other._locations)
    , _isDefault(other._isDefault) {}

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
		_isDefault = other._isDefault;
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

std::string ServerConfig::getEndpoint() const {
	return _host + ":" + Utils::toString(_port);
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

const ServerConfig::ErrorPageMap& ServerConfig::getErrorPages() const {
	return _errorPages;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
	return _locations;
}

std::vector<LocationConfig>& ServerConfig::getLocations() {
	return _locations;
}

bool ServerConfig::isDefault() const {
	return _isDefault;
}

void ServerConfig::setDefault(bool isDefault) {
	_isDefault = isDefault;
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
	if (std::find(_serverNames.begin(), _serverNames.end(), name) ==
	    _serverNames.end()) {
		_serverNames.push_back(name);
	}
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

void ServerConfig::setErrorPages(const ErrorPageMap& pages) {
	_errorPages = pages;
}

void ServerConfig::addErrorPage(int code, const std::string& uri) {
	_errorPages[code] = uri;
}

void ServerConfig::addLocation(const LocationConfig& location) {
	_locations.push_back(location);
}

typedef std::pair<std::string, std::vector<int> > ErrorGroup;
typedef std::vector<ErrorGroup> ErrorGroups;

void ServerConfig::dump(size_t index) const {
	Logger::debug("");
	Logger::debug("[Server " + Utils::toString(index) + "] " + _host + ":" +
	              Utils::toString(_port) + (_isDefault ? " (default)" : ""));

	std::string names = Utils::join(_serverNames, ", ");
	Logger::debug("  Server Names: [" + names + "]");
	Logger::debug("  Client Max Body Size: " +
	              Utils::toString(_clientMaxBodySize) + " bytes");
	Logger::debug("  Root: " + _root);
	Logger::debug("  Index: " + _index);

	if (!_errorPages.empty()) {
		Logger::debug("  Error Pages:");
		ErrorGroups grouped;
		for (ErrorPageMap::const_iterator it = _errorPages.begin();
		     it != _errorPages.end(); ++it) {
			bool found = false;
			for (ErrorGroups::iterator git = grouped.begin();
			     git != grouped.end(); ++git) {
				if (git->first == it->second) {
					git->second.push_back(it->first);
					found = true;
					break;
				}
			}
			if (!found) {
				std::vector<int> codes;
				codes.push_back(it->first);
				grouped.push_back(std::make_pair(it->second, codes));
			}
		}
		for (ErrorGroups::const_iterator git = grouped.begin();
		     git != grouped.end(); ++git) {
			std::string codesStr = "";
			for (std::vector<int>::const_iterator cit = git->second.begin();
			     cit != git->second.end(); ++cit) {
				if (cit != git->second.begin()) codesStr += ", ";
				codesStr += Utils::toString(*cit);
			}
			Logger::debug("    [" + codesStr + "] -> " + git->first);
		}
	}

	Logger::debug("  Locations (" + Utils::toString(_locations.size()) + "):");
	for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
	     it != _locations.end(); ++it) {
		it->dump(_clientMaxBodySize);
	}
}

std::string ServerConfig::getErrorPage(int code) const {
	ErrorPageMap::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end()) {
		return it->second;
	}
	return "";
}

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
	const LocationConfig* bestMatch = NULL;
	size_t longestMatchLen = 0;
	std::string normUri = Utils::normalizePath(uri);

	for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
	     it != _locations.end(); ++it) {
		std::string normLoc = it->getPath();
		if (normUri == normLoc) {
			if (normLoc.length() >= longestMatchLen) {
				longestMatchLen = normLoc.length();
				bestMatch = &(*it);
			}
		} else if (normLoc == "/") {
			if (1 >= longestMatchLen) {
				longestMatchLen = 1;
				bestMatch = &(*it);
			}
		} else if (normUri.find(normLoc) == 0) {
			if (normUri.length() > normLoc.length() &&
			    normUri[normLoc.length()] == '/') {
				if (normLoc.length() >= longestMatchLen) {
					longestMatchLen = normLoc.length();
					bestMatch = &(*it);
				}
			}
		}
	}
	return bestMatch;
}
