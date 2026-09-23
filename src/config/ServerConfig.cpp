/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 21:09:25 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
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
    , _isDefault(false)
    , _configuredDirectives() {}

ServerConfig::ServerConfig(const ServerConfig& other)
    : _host(other._host)
    , _port(other._port)
    , _serverNames(other._serverNames)
    , _clientMaxBodySize(other._clientMaxBodySize)
    , _root(other._root)
    , _index(other._index)
    , _errorPages(other._errorPages)
    , _locations(other._locations)
    , _isDefault(other._isDefault)
    , _configuredDirectives(other._configuredDirectives) {}

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
		_configuredDirectives = other._configuredDirectives;
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
    const LocationConfig* best = NULL;
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
         it != _locations.end(); ++it) {
        const std::string& prefix = it->getPath();
        if (uri.compare(0, prefix.size(), prefix) == 0 &&
            (!best || prefix.size() > best->getPath().size()))
            best = &(*it);
    }
    return best;
}

bool ServerConfig::isSingleDirective(const std::string& name) {
	return (name == "listen" || name == "host" ||
	        name == "client_max_body_size" || name == "root" ||
	        name == "index");
}

void ServerConfig::applyDirective(const std::vector<Token>& tokens) {
	const std::string& name = tokens[0].value;
	if (isSingleDirective(name) && !_configuredDirectives.insert(name).second) {
		throw std::runtime_error("Duplicate directive '" + name + "' on line " +
		                         Utils::toString(tokens[0].line));
	}
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
	if (_host == "localhost") {
		_host = "127.0.0.1";
	}
	int port = Utils::toInt(portStr);
	if (!Utils::isValidPort(port)) {
		throw std::runtime_error("Invalid port '" + portStr + "' on line " +
		                         Utils::toString(tokens[0].line));
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
	if (_host == "localhost") {
		_host = "127.0.0.1";
	}
}

void ServerConfig::handleServerName(const std::vector<Token>& tokens) {
	Utils::assertMinArgs(tokens, 1);
	for (std::vector<Token>::const_iterator it = tokens.begin() + 1;
	     it != tokens.end(); ++it) {
		if (std::find(_serverNames.begin(), _serverNames.end(), it->value) ==
		    _serverNames.end()) {
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
		if (!Utils::isValidErrorCode(code)) {
			throw std::runtime_error("Invalid HTTP error code '" + it->value +
			                         "' on line " +
			                         Utils::toString(tokens[0].line));
		}
		_errorPages[code] = uri;
	}
}
