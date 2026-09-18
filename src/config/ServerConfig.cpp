/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 20:12:14 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <cstddef>
#include <stdexcept>

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

bool ServerConfig::isDefault() const {
	return _isDefault;
}

void ServerConfig::setDefault(bool isDefault) {
	_isDefault = isDefault;
}

LocationConfig& ServerConfig::addLocation(const std::string& path) {
	_locations.push_back(LocationConfig(path));
	return _locations.back();
}

void ServerConfig::finalize() {
	for (std::vector<LocationConfig>::iterator it = _locations.begin();
	     it != _locations.end(); ++it) {
		it->inherit(*this);
	}
}

typedef std::pair<std::string, std::vector<int> > ErrorGroup;
typedef std::vector<ErrorGroup> ErrorGroups;

void ServerConfig::dump(size_t index) const {
	Logger::debug("");
	Logger::debug("[Server " + Utils::toString(index) + "] " + _host + ":" +
	              Utils::toString(_port) + (_isDefault ? " (default)" : ""));

	std::string names = "";
	for (std::vector<std::string>::const_iterator it = _serverNames.begin();
	     it != _serverNames.end(); ++it) {
		if (it != _serverNames.begin()) names += ", ";
		names += *it;
	}
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

static std::string normalizePath(const std::string& p) {
	if (p.length() > 1 && p[p.length() - 1] == '/') {
		return p.substr(0, p.length() - 1);
	}
	return p;
}

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
	const LocationConfig* bestMatch = NULL;
	size_t longestMatchLen = 0;
	std::string normUri = normalizePath(uri);

	for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
	     it != _locations.end(); ++it) {
		std::string normLoc = normalizePath(it->getPath());
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

void ServerConfig::applyDirective(const std::vector<Token>& tokens) {
	const std::string& name = tokens[0].value;
	if (name == "listen")
		handleListen(tokens);
	else if (name == "host")
		handleHost(tokens);
	else if (name == "server_name")
		handleServerName(tokens);
	else if (name == "client_max_body_size")
		handleClientMaxBodySize(tokens);
	else if (name == "root")
		handleRoot(tokens);
	else if (name == "index")
		handleIndex(tokens);
	else if (name == "error_page")
		handleErrorPage(tokens);
	else
		throw std::runtime_error("Unknown directive '" + name + "' on line " +
		                         Utils::toString(tokens[0].line));
}

void ServerConfig::handleListen(const std::vector<Token>& tokens) {
	Utils::assertArgs(tokens, 1);
	std::string arg = tokens[1].value;
	size_t colon = arg.find(':');
	std::string portStr = arg;

	if (colon != std::string::npos) {
		if (colon + 1 >= arg.length()) {
			throw std::runtime_error("Missing port after ':' on line " +
			                         Utils::toString(tokens[0].line));
		}
		if (colon == 0) {
			_host = "127.0.0.1";
		} else {
			_host = arg.substr(0, colon);
		}
		portStr = arg.substr(colon + 1);
	}
	if (!Utils::isValidHost(_host)) {
		throw std::runtime_error("Invalid host '" + _host + "' on line " +
		                         Utils::toString(tokens[0].line));
	}
	int port = Utils::toInt(portStr);
	if (port <= 0 || port > 65535) {
		throw std::runtime_error(
		    "Invalid port on line " + Utils::toString(tokens[0].line));
	}
	_port = port;
}

void ServerConfig::handleHost(const std::vector<Token>& tokens) {
	Utils::assertArgs(tokens, 1);
	if (!Utils::isValidHost(tokens[1].value)) {
		throw std::runtime_error("Invalid host '" + tokens[1].value +
		                         "' on line " +
		                         Utils::toString(tokens[0].line));
	}
	_host = tokens[1].value;
}

void ServerConfig::handleServerName(const std::vector<Token>& tokens) {
	Utils::assertMinArgs(tokens, 1);
	for (std::vector<Token>::const_iterator it = tokens.begin() + 1;
	     it != tokens.end(); ++it) {
		bool exists = false;
		for (std::vector<std::string>::const_iterator sit =
		         _serverNames.begin();
		     sit != _serverNames.end(); ++sit) {
			if (*sit == it->value) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			_serverNames.push_back(it->value);
		}
	}
}

void ServerConfig::handleClientMaxBodySize(const std::vector<Token>& tokens) {
	Utils::assertArgs(tokens, 1);
	_clientMaxBodySize = Utils::parseSize(tokens[1].value);
}

void ServerConfig::handleRoot(const std::vector<Token>& tokens) {
	Utils::assertArgs(tokens, 1);
	_root = tokens[1].value;
}

void ServerConfig::handleIndex(const std::vector<Token>& tokens) {
	Utils::assertArgs(tokens, 1);
	_index = tokens[1].value;
}

void ServerConfig::handleErrorPage(const std::vector<Token>& tokens) {
	Utils::assertMinArgs(tokens, 2);
	std::string uri = tokens.back().value;
	for (std::vector<Token>::const_iterator it = tokens.begin() + 1;
	     it != tokens.end() - 1; ++it) {
		int code = Utils::toInt(it->value);
		if (code < 300 || code > 599) {
			throw std::runtime_error("Invalid HTTP error code '" + it->value +
			                         "' on line " +
			                         Utils::toString(tokens[0].line));
		}
		_errorPages[code] = uri;
	}
}
