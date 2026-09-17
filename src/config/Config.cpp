/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:42:28 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:04:19 by sklaokli         ###   ########.fr       */
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
		throw std::runtime_error("Unable to read configuration file: " + _path);
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
	Logger::info("");
	Logger::info("Total virtual servers: " + Utils::toString(_servers.size()));
	size_t i = 0;
	for (std::vector<ServerConfig>::const_iterator it = _servers.begin();
	     it != _servers.end(); ++it, ++i) {
		it->dump(i);
	}
	Logger::info("");
}
