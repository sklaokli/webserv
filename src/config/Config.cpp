/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:42:28 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:49:01 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Config.hpp"
#include "parser/Parser.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <fstream>
#include <stdexcept>

Config::Config() : _path(""), _servers() {}

Config::Config(const std::string& configPath) : _path(configPath), _servers() {}

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

void Config::parse() {
	Parser parser(_path);
	*this = parser.execute();
	dump();
}

const std::string& Config::getPath() const {
	return _path;
}

const std::vector<ServerConfig>& Config::getServers() const {
	return _servers;
}

void Config::addServer(const ServerConfig& server) {
	_servers.push_back(server);
}

void Config::dump() const {
	Logger::debug("=== Parsed Configuration Summary ===");
	Logger::debug("Total virtual servers: " + Utils::toString(_servers.size()));
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& s = _servers[i];
		Logger::debug("--- Server [" + Utils::toString(i) + "] ---");
		Logger::debug("  Host: " + s.getHost() +
		              " Port: " + Utils::toString(s.getPort()));

		std::string names = "";
		for (size_t j = 0; j < s.getServerNames().size(); ++j) {
			if (j > 0) names += ", ";
			names += s.getServerNames()[j];
		}
		Logger::debug("  Server Names: [" + names + "]");
		Logger::debug("  Client Max Body Size: " +
		              Utils::toString(s.getClientMaxBodySize()) + " bytes");
		Logger::debug("  Root: " + s.getRoot());
		Logger::debug("  Index: " + s.getIndex());

		const std::map<int, std::string>& eps = s.getErrorPages();
		if (!eps.empty()) {
			std::string epStr = "";
			for (std::map<int, std::string>::const_iterator it = eps.begin();
			     it != eps.end(); ++it) {
				epStr += Utils::toString(it->first) + "->" + it->second + " ";
			}
			Logger::debug("  Error Pages: " + epStr);
		}

		Logger::debug(
		    "  Locations (" + Utils::toString(s.getLocations().size()) + "):");
		for (size_t j = 0; j < s.getLocations().size(); ++j) {
			const LocationConfig& l = s.getLocations()[j];
			Logger::debug("    Location: " + l.getPath());
			std::string methods = "";
			for (size_t k = 0; k < l.getAllowedMethods().size(); ++k) {
				if (k > 0) methods += ", ";
				methods += l.getAllowedMethods()[k];
			}
			Logger::debug("      Allowed Methods: [" + methods + "]");
			Logger::debug("      Root: " + l.getRoot());
			Logger::debug("      Index: " + l.getIndex());
			Logger::debug("      Autoindex: " +
			              std::string(l.getAutoindex() ? "on" : "off"));
			if (l.hasRedirect()) {
				Logger::debug(
				    "      Redirect: " + Utils::toString(l.getRedirectCode()) +
				    " -> " + l.getRedirectUrl());
			}
			if (l.getUploadEnable()) {
				Logger::debug("      Upload: enabled (store: " +
				              l.getUploadStore() + ")");
			}
			const std::map<std::string, std::string>& cgis = l.getCgiExt();
			if (!cgis.empty()) {
				std::string cgiStr = "";
				for (std::map<std::string, std::string>::const_iterator it =
				         cgis.begin();
				     it != cgis.end(); ++it) {
					cgiStr += it->first + "->" + it->second + " ";
				}
				Logger::debug("      CGI: " + cgiStr);
			}
		}
	}
	Logger::debug("=====================================");
}
