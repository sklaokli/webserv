/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:52:27 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationConfig.hpp"

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

const std::map<std::string, std::string>& LocationConfig::getCgiExt() const {
	return _cgiExt;
}

size_t LocationConfig::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

void LocationConfig::setPath(const std::string& path) {
	_path = path;
}

void LocationConfig::setAllowedMethods(
    const std::vector<std::string>& methods) {
	_allowedMethods = methods;
}

void LocationConfig::addAllowedMethod(const std::string& method) {
	for (size_t i = 0; i < _allowedMethods.size(); ++i) {
		if (_allowedMethods[i] == method) return;
	}
	_allowedMethods.push_back(method);
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

void LocationConfig::setUploadEnable(bool enable) {
	_uploadEnable = enable;
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

bool LocationConfig::isMethodAllowed(const std::string& method) const {
	if (_allowedMethods.empty()) {
		return method == "GET";
	}
	for (size_t i = 0; i < _allowedMethods.size(); ++i) {
		if (_allowedMethods[i] == method) return true;
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
	std::map<std::string, std::string>::const_iterator it = _cgiExt.find(ext);
	if (it != _cgiExt.end()) {
		return it->second;
	}
	return "";
}
