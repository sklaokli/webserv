/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/17 23:47:52 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <stdexcept>

LocationConfig::LocationConfig()
    : _path("")
    , _allowedMethods()
    , _root("")
    , _index("")
    , _autoindex(false)
    , _redirectCode(0)
    , _redirectUrl("")
    , _uploadEnable(false)
    , _uploadStore("")
    , _cgiExt()
    , _clientMaxBodySize(0) {}

LocationConfig::LocationConfig(const std::string& path)
    : _path(path)
    , _allowedMethods()
    , _root("")
    , _index("")
    , _autoindex(false)
    , _redirectCode(0)
    , _redirectUrl("")
    , _uploadEnable(false)
    , _uploadStore("")
    , _cgiExt()
    , _clientMaxBodySize(0) {}

LocationConfig::LocationConfig(const LocationConfig& other)
    : _path(other._path)
    , _allowedMethods(other._allowedMethods)
    , _root(other._root)
    , _index(other._index)
    , _autoindex(other._autoindex)
    , _redirectCode(other._redirectCode)
    , _redirectUrl(other._redirectUrl)
    , _uploadEnable(other._uploadEnable)
    , _uploadStore(other._uploadStore)
    , _cgiExt(other._cgiExt)
    , _clientMaxBodySize(other._clientMaxBodySize) {}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
	if (this != &other) {
		_path = other._path;
		_allowedMethods = other._allowedMethods;
		_root = other._root;
		_index = other._index;
		_autoindex = other._autoindex;
		_redirectCode = other._redirectCode;
		_redirectUrl = other._redirectUrl;
		_uploadEnable = other._uploadEnable;
		_uploadStore = other._uploadStore;
		_cgiExt = other._cgiExt;
		_clientMaxBodySize = other._clientMaxBodySize;
	}
	return *this;
}

LocationConfig::~LocationConfig() {}

const std::string& LocationConfig::getPath() const {
	return _path;
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const {
	return _allowedMethods;
}

const std::string& LocationConfig::getRoot() const {
	return _root;
}

const std::string& LocationConfig::getIndex() const {
	return _index;
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

int LocationConfig::getRedirectCode() const {
	return _redirectCode;
}

const std::string& LocationConfig::getRedirectUrl() const {
	return _redirectUrl;
}

bool LocationConfig::getUploadEnable() const {
	return _uploadEnable;
}

const std::string& LocationConfig::getUploadStore() const {
	return _uploadStore;
}

const LocationConfig::CgiMap& LocationConfig::getCgiExt() const {
	return _cgiExt;
}

size_t LocationConfig::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

void LocationConfig::inherit(const ServerConfig& server) {
	if (_root.empty()) {
		_root = server.getRoot();
	}
	if (_index.empty()) {
		_index = server.getIndex();
	}
	if (_clientMaxBodySize == 0) {
		_clientMaxBodySize = server.getClientMaxBodySize();
	}
	if (_allowedMethods.empty()) {
		_allowedMethods.push_back("GET");
	}
}

void LocationConfig::dump(size_t serverBodySize) const {
	Logger::info("    Location: " + _path);
	std::string methods = "";
	for (std::vector<std::string>::const_iterator it = _allowedMethods.begin();
	     it != _allowedMethods.end(); ++it) {
		if (it != _allowedMethods.begin()) methods += ", ";
		methods += *it;
	}
	Logger::info("      Allowed Methods: [" + methods + "]");
	Logger::info("      Root: " + _root);
	Logger::info("      Index: " + _index);
	Logger::info("      Autoindex: " + std::string(_autoindex ? "on" : "off"));
	if ((serverBodySize == 0 && _clientMaxBodySize != 0) ||
	    (serverBodySize != 0 && _clientMaxBodySize != serverBodySize)) {
		Logger::info("      Client Max Body Size: " +
		             Utils::toString(_clientMaxBodySize) + " bytes");
	}
	if (hasRedirect()) {
		Logger::info("      Redirect: " + Utils::toString(_redirectCode) +
		             " -> " + _redirectUrl);
	}
	if (_uploadEnable) {
		Logger::info("      Upload: enabled (store: " + _uploadStore + ")");
	}
	if (!_cgiExt.empty()) {
		Logger::info("      CGI Handlers:");
		for (CgiMap::const_iterator it = _cgiExt.begin(); it != _cgiExt.end();
		     ++it) {
			Logger::info("        " + it->first + " -> " + it->second);
		}
	}
}

bool LocationConfig::isMethodAllowed(const std::string& method) const {
	if (_allowedMethods.empty()) {
		return method == "GET";
	}
	for (std::vector<std::string>::const_iterator it = _allowedMethods.begin();
	     it != _allowedMethods.end(); ++it) {
		if (*it == method) return true;
	}
	return false;
}

bool LocationConfig::hasRedirect() const {
	return _redirectCode != 0 && !_redirectUrl.empty();
}

bool LocationConfig::hasCgi(const std::string& ext) const {
	return _cgiExt.find(ext) != _cgiExt.end();
}

std::string LocationConfig::getCgiHandler(const std::string& ext) const {
	CgiMap::const_iterator it = _cgiExt.find(ext);
	if (it != _cgiExt.end()) {
		return it->second;
	}
	return "";
}

static void assertArgs(const std::vector<Token>& tokens, size_t expected) {
	if (tokens.size() - 1 != expected) {
		throw std::runtime_error("Directive '" + tokens[0].value +
		                         "' requires " + Utils::toString(expected) +
		                         (expected == 1 ? " argument" : " arguments") +
		                         " on line " + Utils::toString(tokens[0].line));
	}
}

static void assertMinArgs(
    const std::vector<Token>& tokens, size_t minExpected) {
	if (tokens.size() - 1 < minExpected) {
		throw std::runtime_error(
		    "Directive '" + tokens[0].value + "' requires at least " +
		    Utils::toString(minExpected) +
		    (minExpected == 1 ? " argument" : " arguments") + " on line " +
		    Utils::toString(tokens[0].line));
	}
}

void LocationConfig::applyDirective(const std::vector<Token>& tokens) {
	const std::string& name = tokens[0].value;
	if (name == "allow_methods")
		handleAllowMethods(tokens);
	else if (name == "root")
		handleRoot(tokens);
	else if (name == "index")
		handleIndex(tokens);
	else if (name == "autoindex")
		handleAutoindex(tokens);
	else if (name == "return")
		handleReturn(tokens);
	else if (name == "client_max_body_size")
		handleClientMaxBodySize(tokens);
	else if (name == "upload_enable")
		handleUploadEnable(tokens);
	else if (name == "upload_store")
		handleUploadStore(tokens);
	else if (name == "cgi_ext")
		handleCgiExt(tokens);
	else
		throw std::runtime_error("Unknown directive '" + name +
		                         "' in location on line " +
		                         Utils::toString(tokens[0].line));
}

void LocationConfig::handleAllowMethods(const std::vector<Token>& tokens) {
	assertMinArgs(tokens, 1);
	for (std::vector<Token>::const_iterator it = tokens.begin() + 1;
	     it != tokens.end(); ++it) {
		bool exists = false;
		for (std::vector<std::string>::const_iterator mit =
		         _allowedMethods.begin();
		     mit != _allowedMethods.end(); ++mit) {
			if (*mit == it->value) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			_allowedMethods.push_back(it->value);
		}
	}
}

void LocationConfig::handleRoot(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	_root = tokens[1].value;
}

void LocationConfig::handleIndex(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	_index = tokens[1].value;
}

void LocationConfig::handleAutoindex(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	if (tokens[1].value != "on" && tokens[1].value != "off") {
		throw std::runtime_error(
		    "Directive 'autoindex' must be 'on' or 'off' on line " +
		    Utils::toString(tokens[0].line));
	}
	_autoindex = (tokens[1].value == "on");
}

void LocationConfig::handleReturn(const std::vector<Token>& tokens) {
	if (tokens.size() == 3) {
		_redirectCode = Utils::toInt(tokens[1].value);
		_redirectUrl = tokens[2].value;
	} else if (tokens.size() == 2) {
		_redirectCode = 302;
		_redirectUrl = tokens[1].value;
	} else {
		throw std::runtime_error(
		    "Directive 'return' requires 1 or 2 arguments on line " +
		    Utils::toString(tokens[0].line));
	}
}

void LocationConfig::handleClientMaxBodySize(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	_clientMaxBodySize = Utils::parseSize(tokens[1].value);
}

void LocationConfig::handleUploadEnable(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	if (tokens[1].value != "on" && tokens[1].value != "off") {
		throw std::runtime_error(
		    "Directive 'upload_enable' must be 'on' or 'off' on line " +
		    Utils::toString(tokens[0].line));
	}
	_uploadEnable = (tokens[1].value == "on");
}

void LocationConfig::handleUploadStore(const std::vector<Token>& tokens) {
	assertArgs(tokens, 1);
	_uploadStore = tokens[1].value;
}

void LocationConfig::handleCgiExt(const std::vector<Token>& tokens) {
	assertArgs(tokens, 2);
	_cgiExt[tokens[1].value] = tokens[2].value;
}
