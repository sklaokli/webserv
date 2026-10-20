/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:13:48 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:39:31 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Parser.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <stdexcept>

Directive::Directive() : name(""), args(), line(0) {}

Directive::Directive(const std::string& n, size_t l)
    : name(n), args(), line(l) {}

Parser::Parser() : _filePath("conf/default.conf"), _tokens(), _it(), _end() {}

Parser::Parser(const std::string& filePath)
    : _filePath(filePath), _tokens(), _it(), _end() {}

Parser::Parser(const Parser& other)
    : _filePath(other._filePath)
    , _tokens(other._tokens)
    , _it(_tokens.begin())
    , _end(_tokens.end()) {
	if (!other._tokens.empty() && other._it != other._tokens.end()) {
		size_t offset = other._it - other._tokens.begin();
		_it = _tokens.begin() + offset;
	}
}

Parser& Parser::operator=(const Parser& other) {
	if (this != &other) {
		_filePath = other._filePath;
		_tokens = other._tokens;
		_it = _tokens.begin();
		_end = _tokens.end();
		if (!other._tokens.empty() && other._it != other._tokens.end()) {
			size_t offset = other._it - other._tokens.begin();
			_it = _tokens.begin() + offset;
		}
	}
	return *this;
}

Parser::~Parser() {}

Config Parser::parseConfig() {
	if (_filePath.empty()) {
		_filePath = "conf/default.conf";
	}
	return parseConfig(_filePath);
}

Config Parser::parseConfig(const std::string& filePath) {
	Config config;
	parse(filePath, config);
	return config;
}

const std::string& Parser::getFilePath() const {
	return _filePath;
}

bool Parser::parseArgs(int argc, char** argv) {
	std::string filePath = "conf/default.conf";
	bool configSet = false;
	bool debugSet = false;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--debug" || arg == "-d") {
			if (debugSet) {
				Logger::error("Duplicate option '" + arg + "'");
				return false;
			}
			Logger::setLogLevel(Logger::DEBUG);
			debugSet = true;
		} else if (!arg.empty() && arg[0] == '-') {
			Logger::error("Unknown option '" + arg + "'");
			return false;
		} else if (!configSet) {
			filePath = arg;
			configSet = true;
		} else {
			Logger::error("Too many arguments");
			return false;
		}
	}
	_filePath = filePath;
	return true;
}

void Parser::parse(const std::string& filePath, Config& config) {
	_filePath = filePath;
	std::string content = Utils::readFile(_filePath);
	if (content.empty()) {
		throw std::runtime_error("Configuration file is empty: " + _filePath);
	}
	Logger::debug("Read " + Utils::toString(content.length()) + " bytes from " +
	              _filePath);

	_tokens = Lexer::tokenize(content);
	_it = _tokens.begin();
	_end = _tokens.end();

	config.setPath(_filePath);
	while (!isDone()) {
		parseServer(config);
	}

	if (config.getServers().empty()) {
		throw std::runtime_error("No server blocks found in " + _filePath);
	}

	validate(config);
	Logger::info("Parsed configuration: " + _filePath);
	Logger::info("Loaded " + Utils::toString(config.getServers().size()) +
	             " virtual server(s)");
	config.dump();

	_tokens.clear();
	_it = _tokens.end();
	_end = _tokens.end();
}

bool Parser::isDone() const {
	return _it == _end;
}

void Parser::expect(const std::string& expected) {
	if (_it == _end) {
		throw std::runtime_error(
		    "Reached end of file while expecting '" + expected + "'");
	}

	if (_it->value != expected) {
		std::string line = Utils::toString(_it->line);
		std::string token = _it->value;
		throw std::runtime_error("Expected '" + expected + "' on line " + line +
		                         ", found '" + token + "'");
	}

	++_it;
}

Directive Parser::readDirective() {
	if (_it == _end) {
		throw std::runtime_error("Unexpected EOF while reading directive");
	}
	Directive dir(_it->value, _it->line);
	++_it;

	if (_it == _end || _it->value == ";" || _it->value == "{" ||
	    _it->value == "}") {
		throw std::runtime_error("Unexpected '" +
		                         (_it != _end ? _it->value : "EOF") +
		                         "' on line " + Utils::toString(dir.line));
	}

	while (_it != _end && _it->value != ";" && _it->value != "{" &&
	       _it->value != "}") {
		dir.args.push_back(_it->value);
		++_it;
	}

	expect(";");
	return dir;
}

void Parser::parseServer(Config& config) {
	if (_it == _end) {
		throw std::runtime_error(
		    "Reached end of file while expecting 'server'");
	}
	expect("server");
	expect("{");

	ServerConfig server;
	std::set<std::string> configured;
	std::vector<LocationConfig> locations;

	while (_it != _end && _it->value != "}") {
		if (_it->value == "location") {
			parseLocation(locations);
		} else {
			parseServerDirective(server, configured);
		}
	}

	expect("}");

	for (size_t i = 0; i < locations.size(); ++i) {
		inheritLocation(locations[i], server);
		server.addLocation(locations[i]);
	}

	config.addServer(server);
}

void Parser::checkSingle(
    const std::string& name, size_t line, std::set<std::string>& configured) {
	if (!configured.insert(name).second) {
		throw std::runtime_error("Duplicate directive '" + name + "' on line " +
		                         Utils::toString(line));
	}
}

void Parser::parseServerDirective(
    ServerConfig& server, std::set<std::string>& configured) {
	Directive dir = readDirective();
	if (dir.name == "listen") {
		checkSingle(dir.name, dir.line, configured);
		parseListen(dir, server);
	} else if (dir.name == "host") {
		checkSingle(dir.name, dir.line, configured);
		parseHost(dir, server);
	} else if (dir.name == "server_name") {
		parseServerName(dir, server);
	} else if (dir.name == "client_max_body_size") {
		checkSingle(dir.name, dir.line, configured);
		server.setClientMaxBodySize(parseClientMaxBodySize(dir));
	} else if (dir.name == "root") {
		checkSingle(dir.name, dir.line, configured);
		server.setRoot(parseRoot(dir));
	} else if (dir.name == "index") {
		checkSingle(dir.name, dir.line, configured);
		server.setIndex(parseIndex(dir));
	} else if (dir.name == "error_page") {
		parseErrorPage(dir, server);
	} else {
		throw std::runtime_error("Unknown directive '" + dir.name +
		                         "' on line " + Utils::toString(dir.line));
	}
}

void Parser::parseListen(const Directive& dir, ServerConfig& server) {
	assertArgs(dir, 1);
	std::string arg = dir.args[0];
	size_t colon = arg.find(':');
	std::string host = server.getHost();
	std::string portStr = arg;

	if (colon != std::string::npos) {
		if (colon + 1 >= arg.length()) {
			throw std::runtime_error(
			    "Missing port after ':' on line " + Utils::toString(dir.line));
		}
		if (colon == 0) {
			host = "127.0.0.1";
		} else {
			host = arg.substr(0, colon);
		}
		portStr = arg.substr(colon + 1);
	}
	if (!isValidHost(host)) {
		throw std::runtime_error(
		    "Invalid host '" + host + "' on line " + Utils::toString(dir.line));
	}
	if (host == "localhost") {
		host = "127.0.0.1";
	}
	int port = Utils::toInt(portStr);
	if (!isValidPort(port)) {
		throw std::runtime_error("Invalid port '" + portStr + "' on line " +
		                         Utils::toString(dir.line));
	}
	server.setHost(host);
	server.setPort(port);
}

void Parser::parseHost(const Directive& dir, ServerConfig& server) {
	assertArgs(dir, 1);
	if (!isValidHost(dir.args[0])) {
		throw std::runtime_error("Invalid host '" + dir.args[0] + "' on line " +
		                         Utils::toString(dir.line));
	}
	std::string host = dir.args[0];
	if (host == "localhost") {
		host = "127.0.0.1";
	}
	server.setHost(host);
}

void Parser::parseServerName(const Directive& dir, ServerConfig& server) {
	assertMinArgs(dir, 1);
	for (std::vector<std::string>::const_iterator it = dir.args.begin();
	     it != dir.args.end(); ++it) {
		server.addServerName(*it);
	}
}

void Parser::parseErrorPage(const Directive& dir, ServerConfig& server) {
	assertMinArgs(dir, 2);
	std::string uri = dir.args.back();
	for (std::vector<std::string>::const_iterator it = dir.args.begin();
	     it != dir.args.end() - 1; ++it) {
		int code = Utils::toInt(*it);
		if (!isValidErrorCode(code)) {
			throw std::runtime_error("Invalid HTTP error code '" + *it +
			                         "' on line " + Utils::toString(dir.line));
		}
		server.addErrorPage(code, uri);
	}
}

void Parser::parseLocation(std::vector<LocationConfig>& locations) {
	if (_it == _end) {
		throw std::runtime_error(
		    "Reached end of file while expecting 'location'");
	}
	size_t line = _it->line;
	expect("location");

	if (_it == _end || _it->value == "{" || _it->value == ";") {
		throw std::runtime_error(
		    "Missing location path on line " + Utils::toString(line));
	}

	std::string path = _it->value;
	if (path.empty() || path[0] != '/') {
		throw std::runtime_error("Location path must start with '/' on line " +
		                         Utils::toString(_it->line));
	}
	++_it;

	expect("{");

	LocationConfig loc(path);
	std::set<std::string> configured;

	while (_it != _end && _it->value != "}") {
		parseLocationDirective(loc, configured);
	}

	expect("}");

	locations.push_back(loc);
}

void Parser::parseLocationDirective(
    LocationConfig& loc, std::set<std::string>& configured) {
	Directive dir = readDirective();
	if (dir.name == "allow_methods") {
		checkSingle(dir.name, dir.line, configured);
		parseAllowMethods(dir, loc);
	} else if (dir.name == "root") {
		checkSingle(dir.name, dir.line, configured);
		loc.setRoot(parseRoot(dir));
	} else if (dir.name == "index") {
		checkSingle(dir.name, dir.line, configured);
		loc.setIndex(parseIndex(dir));
	} else if (dir.name == "autoindex") {
		checkSingle(dir.name, dir.line, configured);
		parseAutoindex(dir, loc);
	} else if (dir.name == "return") {
		checkSingle(dir.name, dir.line, configured);
		parseReturn(dir, loc);
	} else if (dir.name == "client_max_body_size") {
		checkSingle(dir.name, dir.line, configured);
		loc.setClientMaxBodySize(parseClientMaxBodySize(dir));
	} else if (dir.name == "upload_enable") {
		checkSingle(dir.name, dir.line, configured);
		parseUploadEnable(dir, loc);
	} else if (dir.name == "upload_store") {
		checkSingle(dir.name, dir.line, configured);
		parseUploadStore(dir, loc);
	} else if (dir.name == "cgi_ext") {
		parseCgiExt(dir, loc);
	} else {
		throw std::runtime_error("Unknown directive '" + dir.name +
		                         "' in location on line " +
		                         Utils::toString(dir.line));
	}
}

void Parser::parseAllowMethods(const Directive& dir, LocationConfig& loc) {
	assertMinArgs(dir, 1);
	for (std::vector<std::string>::const_iterator it = dir.args.begin();
	     it != dir.args.end(); ++it) {
		if (!isValidMethod(*it)) {
			throw std::runtime_error("Invalid HTTP method '" + *it +
			                         "' on line " + Utils::toString(dir.line));
		}
		loc.addAllowedMethod(*it);
	}
}

void Parser::parseAutoindex(const Directive& dir, LocationConfig& loc) {
	assertArgs(dir, 1);
	if (dir.args[0] != "on" && dir.args[0] != "off") {
		throw std::runtime_error(
		    "Directive 'autoindex' must be 'on' or 'off' on line " +
		    Utils::toString(dir.line));
	}
	loc.setAutoindex(dir.args[0] == "on");
}

void Parser::parseReturn(const Directive& dir, LocationConfig& loc) {
	assertArgs(dir, 1, 2);
	if (dir.args.size() == 2) {
		int code = Utils::toInt(dir.args[0]);
		if (!isValidRedirectCode(code)) {
			throw std::runtime_error("Invalid redirect status code '" +
			                         dir.args[0] + "' on line " +
			                         Utils::toString(dir.line));
		}
		loc.setRedirect(code, dir.args[1]);
	} else {
		loc.setRedirect(LocationConfig::StatusCodes::DEFAULT, dir.args[0]);
	}
}

void Parser::parseUploadEnable(const Directive& dir, LocationConfig& loc) {
	assertArgs(dir, 1);
	if (dir.args[0] != "on" && dir.args[0] != "off") {
		throw std::runtime_error(
		    "Directive 'upload_enable' must be 'on' or 'off' on line " +
		    Utils::toString(dir.line));
	}
	loc.setUploadEnable(dir.args[0] == "on");
}

void Parser::parseUploadStore(const Directive& dir, LocationConfig& loc) {
	assertArgs(dir, 1);
	loc.setUploadStore(dir.args[0]);
}

void Parser::parseCgiExt(const Directive& dir, LocationConfig& loc) {
	assertArgs(dir, 2);
	std::string ext = dir.args[0];
	if (ext.empty()) {
		throw std::runtime_error(
		    "Empty CGI extension on line " + Utils::toString(dir.line));
	}
	if (ext[0] != '.') {
		ext = "." + ext;
	}
	if (loc.getCgiExt().find(ext) != loc.getCgiExt().end()) {
		throw std::runtime_error("Duplicate CGI extension '" + ext +
		                         "' on line " + Utils::toString(dir.line));
	}
	loc.addCgiExt(ext, dir.args[1]);
}

std::string Parser::parseRoot(const Directive& dir) {
	assertArgs(dir, 1);
	return dir.args[0];
}

std::string Parser::parseIndex(const Directive& dir) {
	assertArgs(dir, 1);
	return dir.args[0];
}

size_t Parser::parseClientMaxBodySize(const Directive& dir) {
	assertArgs(dir, 1);
	return parseSize(dir.args[0]);
}

size_t Parser::parseSize(const std::string& str) {
	if (str.empty()) {
		throw std::runtime_error("Empty size string");
	}
	char unit = str[str.length() - 1];
	size_t multiplier = 1;
	std::string numStr = str;

	if (unit == 'k' || unit == 'K') {
		multiplier = Config::SizeUnits::KB;
		numStr = str.substr(0, str.length() - 1);
	} else if (unit == 'm' || unit == 'M') {
		multiplier = Config::SizeUnits::MB;
		numStr = str.substr(0, str.length() - 1);
	} else if (unit == 'g' || unit == 'G') {
		multiplier = Config::SizeUnits::GB;
		numStr = str.substr(0, str.length() - 1);
	}

	if (numStr.empty()) {
		throw std::runtime_error("Invalid size: '" + str + "'");
	}
	for (size_t i = 0; i < numStr.length(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(numStr[i]))) {
			throw std::runtime_error(
			    "Invalid character in size: '" + str + "'");
		}
	}

	std::istringstream iss(numStr);
	size_t baseVal;
	if (!(iss >> baseVal) || !iss.eof()) {
		throw std::runtime_error("Invalid size value: '" + str + "'");
	}
	if (multiplier > 1 && baseVal > (static_cast<size_t>(-1) / multiplier)) {
		throw std::runtime_error(
		    "Size value causes integer overflow: '" + str + "'");
	}
	return baseVal * multiplier;
}

void Parser::inheritLocation(LocationConfig& loc, const ServerConfig& server) {
	if (loc.getRoot().empty()) {
		loc.setRoot(server.getRoot());
	}
	if (loc.getIndex().empty()) {
		loc.setIndex(server.getIndex());
	}
	if (loc.getClientMaxBodySize() == 0) {
		loc.setClientMaxBodySize(server.getClientMaxBodySize());
	}
	if (loc.getAllowedMethods().empty()) {
		loc.addAllowedMethod("GET");
	}
}

void Parser::validate(Config& config) {
	std::set<std::string> seenEndpoints;
	std::vector<ServerConfig>& servers = config.getServers();

	for (std::vector<ServerConfig>::iterator sit = servers.begin();
	     sit != servers.end(); ++sit) {
		std::string endpoint = sit->getEndpoint();

		if (seenEndpoints.insert(endpoint).second) {
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

	for (size_t i = 0; i < servers.size(); ++i) {
		std::string ep1 = servers[i].getEndpoint();
		const std::vector<std::string>& names1 = servers[i].getServerNames();

		for (size_t j = i + 1; j < servers.size(); ++j) {
			std::string ep2 = servers[j].getEndpoint();
			if (ep1 != ep2) continue;

			const std::vector<std::string>& names2 =
			    servers[j].getServerNames();

			if (names1.empty() && names2.empty()) {
				throw std::runtime_error(
				    "Conflicting default server (duplicate unnamed server) "
				    "on " +
				    ep1);
			}

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

bool Parser::isValidHost(const std::string& host) {
	if (host.empty()) return false;
	if (host == "localhost") return true;

	size_t start = 0;
	size_t dots = 0;

	for (size_t i = 0; i <= host.length(); ++i) {
		if (i == host.length() || host[i] == '.') {
			if (i == start || i - start > 3) return false;
			std::string octetStr = host.substr(start, i - start);
			for (size_t j = 0; j < octetStr.length(); ++j) {
				if (!std::isdigit(static_cast<unsigned char>(octetStr[j]))) {
					return false;
				}
			}
			int octet = Utils::toInt(octetStr);
			if (octet < Config::IpV4::MIN || octet > Config::IpV4::MAX) {
				return false;
			}
			if (i < host.length()) {
				++dots;
				start = i + 1;
			}
		}
	}
	return dots == 3;
}

bool Parser::isValidPort(int port) {
	return port >= ServerConfig::Ports::MIN &&
	       port <= ServerConfig::Ports::MAX;
}

bool Parser::isValidMethod(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE" ||
	        method == "HEAD" || method == "PUT");
}

bool Parser::isValidErrorCode(int code) {
	return code >= ServerConfig::StatusCodes::MIN &&
	       code <= ServerConfig::StatusCodes::MAX;
}

bool Parser::isValidRedirectCode(int code) {
	return code >= LocationConfig::StatusCodes::MIN &&
	       code <= LocationConfig::StatusCodes::MAX;
}

void Parser::assertArgs(const Directive& dir, size_t expected) {
	if (dir.args.size() != expected) {
		throw std::runtime_error("Directive '" + dir.name + "' requires " +
		                         Utils::toString(expected) +
		                         (expected == 1 ? " argument" : " arguments") +
		                         " on line " + Utils::toString(dir.line));
	}
}

void Parser::assertArgs(const Directive& dir, size_t min, size_t max) {
	size_t count = dir.args.size();
	if (count < min || count > max) {
		std::string rangeStr;
		if (min == max) {
			rangeStr =
			    Utils::toString(min) + (min == 1 ? " argument" : " arguments");
		} else if (max == min + 1) {
			rangeStr = Utils::toString(min) + " or " + Utils::toString(max) +
			           " arguments";
		} else {
			rangeStr = Utils::toString(min) + " to " + Utils::toString(max) +
			           " arguments";
		}
		throw std::runtime_error("Directive '" + dir.name + "' requires " +
		                         rangeStr + " on line " +
		                         Utils::toString(dir.line));
	}
}

void Parser::assertMinArgs(const Directive& dir, size_t min) {
	if (dir.args.size() < min) {
		throw std::runtime_error("Directive '" + dir.name +
		                         "' requires at least " + Utils::toString(min) +
		                         (min == 1 ? " argument" : " arguments") +
		                         " on line " + Utils::toString(dir.line));
	}
}
