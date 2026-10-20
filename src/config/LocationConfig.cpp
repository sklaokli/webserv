/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:44:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

const int LocationConfig::StatusCodes::MIN;
const int LocationConfig::StatusCodes::MAX;
const int LocationConfig::StatusCodes::DEFAULT;

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
    : _path(Utils::normalizePath(path))
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

void LocationConfig::setPath(const std::string& path) {
	_path = Utils::normalizePath(path);
}

void LocationConfig::setAllowedMethods(const std::vector<std::string>& methods) {
	_allowedMethods = methods;
}

void LocationConfig::addAllowedMethod(const std::string& method) {
	if (std::find(_allowedMethods.begin(), _allowedMethods.end(), method) ==
	    _allowedMethods.end()) {
		_allowedMethods.push_back(method);
	}
}

void LocationConfig::setRoot(const std::string& root) {
	_root = root;
}

void LocationConfig::setIndex(const std::string& index) {
	_index = index;
}

void LocationConfig::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

void LocationConfig::setRedirect(int code, const std::string& url) {
	_redirectCode = code;
	_redirectUrl = url;
}

void LocationConfig::setUploadEnable(bool uploadEnable) {
	_uploadEnable = uploadEnable;
}

void LocationConfig::setUploadStore(const std::string& store) {
	_uploadStore = store;
}

void LocationConfig::addCgiExt(
    const std::string& ext, const std::string& handler) {
	_cgiExt[ext] = handler;
}

void LocationConfig::setClientMaxBodySize(size_t size) {
	_clientMaxBodySize = size;
}

void LocationConfig::dump(size_t serverBodySize) const {
	Logger::debug("    Location: " + _path);
	std::string methods = Utils::join(_allowedMethods, ", ");
	Logger::debug("      Allowed Methods: [" + methods + "]");
	Logger::debug("      Root: " + _root);
	Logger::debug("      Index: " + _index);
	Logger::debug("      Autoindex: " + std::string(_autoindex ? "on" : "off"));
	if ((serverBodySize == 0 && _clientMaxBodySize != 0) ||
	    (serverBodySize != 0 && _clientMaxBodySize != serverBodySize)) {
		Logger::debug("      Client Max Body Size: " +
		              Utils::toString(_clientMaxBodySize) + " bytes");
	}
	if (hasRedirect()) {
		Logger::debug("      Redirect: " + Utils::toString(_redirectCode) +
		              " -> " + _redirectUrl);
	}
	if (_uploadEnable) {
		Logger::debug("      Upload: enabled (store: " + _uploadStore + ")");
	}
	if (!_cgiExt.empty()) {
		Logger::debug("      CGI Handlers:");
		for (CgiMap::const_iterator it = _cgiExt.begin(); it != _cgiExt.end();
		     ++it) {
			Logger::debug("        " + it->first + " -> " + it->second);
		}
	}
}

bool LocationConfig::isMethodAllowed(const std::string& method) const {
	if (_allowedMethods.empty()) {
		return method == "GET";
	}
	return std::find(_allowedMethods.begin(), _allowedMethods.end(), method) !=
	       _allowedMethods.end();
}

bool LocationConfig::hasRedirect() const {
	return _redirectCode != 0 && !_redirectUrl.empty();
}

bool LocationConfig::hasCgi(const std::string& ext) const {
	return !getCgiHandler(ext).empty();
}

std::string LocationConfig::getCgiHandler(const std::string& ext) const {
	CgiMap::const_iterator it = _cgiExt.find(ext);
	if (it != _cgiExt.end()) {
		return it->second;
	}
	return "";
}
