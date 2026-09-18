/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:42:28 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 20:04:30 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Config.hpp"
#include "parser/Lexer.hpp"
#include "parser/Parser.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
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
	std::string content = Utils::readFile(_path);
	if (content.empty()) {
		throw std::runtime_error("Configuration file is empty: " + _path);
	}
	Logger::debug(
	    "Read " + Utils::toString(content.length()) + " bytes from " + _path);

	std::vector<Token> tokens = Lexer::tokenize(content);

	Parser parser(tokens);
	std::vector<Token>::const_iterator it = tokens.begin();
	while (it != tokens.end()) {
		ServerConfig& server = addServer();
		parser.parseServer(server, it);
	}
	if (_servers.empty()) {
		throw std::runtime_error("No server blocks found in " + _path);
	}

	validate();
	Logger::info("Parsed configuration: " + _path);
	Logger::info(
	    "Loaded " + Utils::toString(_servers.size()) + " virtual server(s)");
	dump();
}

const std::string& Config::getPath() const {
	return _path;
}

const std::vector<ServerConfig>& Config::getServers() const {
	return _servers;
}

ServerConfig& Config::addServer() {
	_servers.push_back(ServerConfig());
	return _servers.back();
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

void Config::validate() {
	std::vector<std::string> seenEndpoints;

	for (std::vector<ServerConfig>::iterator sit = _servers.begin();
	     sit != _servers.end(); ++sit) {
		std::string endpoint =
		    sit->getHost() + ":" + Utils::toString(sit->getPort());

		bool firstForEndpoint = true;
		for (size_t i = 0; i < seenEndpoints.size(); ++i) {
			if (seenEndpoints[i] == endpoint) {
				firstForEndpoint = false;
				break;
			}
		}

		if (firstForEndpoint) {
			seenEndpoints.push_back(endpoint);
			sit->setDefault(true);
		}

		const std::vector<LocationConfig>& locs = sit->getLocations();
		for (size_t i = 0; i < locs.size(); ++i) {
			for (size_t j = i + 1; j < locs.size(); ++j) {
				if (locs[i].getPath() == locs[j].getPath()) {
					throw std::runtime_error("Duplicate location path '" +
					                         locs[i].getPath() +
					                         "' in server " + endpoint);
				}
			}
		}
	}

	for (size_t i = 0; i < _servers.size(); ++i) {
		std::string ep1 = _servers[i].getHost() + ":" +
		                  Utils::toString(_servers[i].getPort());
		const std::vector<std::string>& names1 = _servers[i].getServerNames();

		for (size_t j = i + 1; j < _servers.size(); ++j) {
			std::string ep2 = _servers[j].getHost() + ":" +
			                  Utils::toString(_servers[j].getPort());
			if (ep1 != ep2) continue;

			const std::vector<std::string>& names2 =
			    _servers[j].getServerNames();
			for (size_t n1 = 0; n1 < names1.size(); ++n1) {
				for (size_t n2 = 0; n2 < names2.size(); ++n2) {
					if (names1[n1] == names2[n2]) {
						throw std::runtime_error("Conflicting server_name '" +
						                         names1[n1] + "' on " + ep1);
					}
				}
			}
		}
	}
}
