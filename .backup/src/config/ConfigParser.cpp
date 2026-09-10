#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <fstream>
#include <stdexcept>

ConfigParser::ConfigParser() {}
ConfigParser::~ConfigParser() {}

std::vector<std::string> ConfigParser::tokenize(const std::string& content) {
	std::vector<std::string> tokens;
	std::string current;
	bool inComment = false;

	for (size_t i = 0; i < content.length(); ++i) {
		char c = content[i];

		if (inComment) {
			if (c == '\n') inComment = false;
			continue;
		}

		if (c == '#') {
			inComment = true;
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, c));
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(c))) {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}

		current += c;
	}

	if (!current.empty()) {
		tokens.push_back(current);
	}

	return tokens;
}

Config ConfigParser::parse(const std::string& filePath) {
	std::string content = Utils::readFileToString(filePath);
	if (content.empty() && !Utils::fileExists(filePath)) {
		throw std::runtime_error(
		    "Failed to open or read configuration file: " + filePath);
	}

	std::vector<std::string> tokens = tokenize(content);
	Config config;
	size_t index = 0;

	while (index < tokens.size()) {
		if (tokens[index] == "server") {
			index++;
			if (index >= tokens.size() || tokens[index] != "{") {
				throw std::runtime_error("Expected '{' after 'server'");
			}
			index++;
			parseServer(tokens, index, config);
		} else {
			throw std::runtime_error(
			    "Unexpected token outside server block: " + tokens[index]);
		}
	}

	if (config.getServers().empty()) {
		throw std::runtime_error("Configuration contains no server blocks");
	}

	return config;
}

void ConfigParser::parseServer(
    const std::vector<std::string>& tokens, size_t& index, Config& config) {
	ServerConfig server;

	while (index < tokens.size() && tokens[index] != "}") {
		if (tokens[index] == "location") {
			index++;
			parseLocation(tokens, index, server);
		} else {
			parseDirective(tokens, index, server);
		}
	}

	if (index >= tokens.size() || tokens[index] != "}") {
		throw std::runtime_error("Unclosed server block: missing '}'");
	}
	index++;  // consume '}'

	// If server has no location blocks, add a default "/"
	if (server.locations.empty()) {
		LocationConfig defaultLoc;
		defaultLoc.path = "/";
		defaultLoc.root = server.root;
		defaultLoc.index = server.index;
		server.locations.push_back(defaultLoc);
	} else {
		// Inherit root/index if not set in location
		for (size_t i = 0; i < server.locations.size(); ++i) {
			if (server.locations[i].root.empty())
				server.locations[i].root = server.root;
			if (server.locations[i].index.empty())
				server.locations[i].index = server.index;
		}
	}

	config.addServer(server);
}

void ConfigParser::parseDirective(const std::vector<std::string>& tokens,
    size_t& index, ServerConfig& server) {
	std::string directive = tokens[index++];
	std::vector<std::string> args;

	while (index < tokens.size() && tokens[index] != ";" &&
	       tokens[index] != "{" && tokens[index] != "}") {
		args.push_back(tokens[index++]);
	}

	if (index >= tokens.size() || tokens[index] != ";") {
		throw std::runtime_error(
		    "Directive '" + directive + "' must end with ';'");
	}
	index++;  // consume ';'

	if (args.empty()) {
		throw std::runtime_error(
		    "Directive '" + directive + "' has no arguments");
	}

	if (directive == "listen") {
		std::string val = args[0];
		size_t colon = val.find(':');
		if (colon != std::string::npos) {
			server.host = val.substr(0, colon);
			server.port = Utils::toInt(val.substr(colon + 1));
		} else {
			server.port = Utils::toInt(val);
		}
		if (server.port <= 0 || server.port > 65535) {
			throw std::runtime_error(
			    "Invalid port in listen directive: " + val);
		}
	} else if (directive == "host") {
		server.host = args[0];
	} else if (directive == "server_name") {
		for (size_t i = 0; i < args.size(); ++i) {
			server.server_names.push_back(args[i]);
		}
	} else if (directive == "root") {
		server.root = args[0];
	} else if (directive == "index") {
		server.index = args[0];
	} else if (directive == "client_max_body_size") {
		server.client_max_body_size = Utils::parseSize(args[0]);
	} else if (directive == "error_page") {
		if (args.size() < 2) {
			throw std::runtime_error(
			    "error_page requires at least one error code and a URI");
		}
		std::string pageUri = args[args.size() - 1];
		for (size_t i = 0; i < args.size() - 1; ++i) {
			int code = Utils::toInt(args[i]);
			if (code >= 400 && code <= 599) {
				server.error_pages[code] = pageUri;
			} else {
				throw std::runtime_error("Invalid HTTP error code: " + args[i]);
			}
		}
	} else {
		throw std::runtime_error(
		    "Unknown directive in server block: " + directive);
	}
}

void ConfigParser::parseLocation(const std::vector<std::string>& tokens,
    size_t& index, ServerConfig& server) {
	if (index >= tokens.size()) {
		throw std::runtime_error("Expected path after 'location'");
	}
	LocationConfig location;
	location.path = tokens[index++];

	if (index >= tokens.size() || tokens[index] != "{") {
		throw std::runtime_error(
		    "Expected '{' after location path " + location.path);
	}
	index++;  // consume '{'

	while (index < tokens.size() && tokens[index] != "}") {
		parseLocationDirective(tokens, index, location);
	}

	if (index >= tokens.size() || tokens[index] != "}") {
		throw std::runtime_error(
		    "Unclosed location block for " + location.path);
	}
	index++;  // consume '}'

	server.locations.push_back(location);
}

void ConfigParser::parseLocationDirective(
    const std::vector<std::string>& tokens, size_t& index,
    LocationConfig& location) {
	std::string directive = tokens[index++];
	std::vector<std::string> args;

	while (index < tokens.size() && tokens[index] != ";" &&
	       tokens[index] != "{" && tokens[index] != "}") {
		args.push_back(tokens[index++]);
	}

	if (index >= tokens.size() || tokens[index] != ";") {
		throw std::runtime_error(
		    "Location directive '" + directive + "' must end with ';'");
	}
	index++;  // consume ';'

	if (args.empty()) {
		throw std::runtime_error(
		    "Location directive '" + directive + "' has no arguments");
	}

	if (directive == "root") {
		location.root = args[0];
	} else if (directive == "index") {
		location.index = args[0];
	} else if (directive == "allow_methods" || directive == "allowed_methods") {
		location.allowed_methods.clear();
		for (size_t i = 0; i < args.size(); ++i) {
			location.allowed_methods.insert(args[i]);
		}
	} else if (directive == "autoindex") {
		location.autoindex =
		    (args[0] == "on" || args[0] == "true" || args[0] == "1");
	} else if (directive == "return") {
		if (args.size() == 1) {
			location.redirect_code = 302;
			location.redirect_url = args[0];
		} else if (args.size() >= 2) {
			location.redirect_code = Utils::toInt(args[0]);
			location.redirect_url = args[1];
		}
	} else if (directive == "upload_enable") {
		location.upload_enable =
		    (args[0] == "on" || args[0] == "true" || args[0] == "1");
	} else if (directive == "upload_store") {
		location.upload_store = args[0];
		location.upload_enable = true;
	} else if (directive == "client_max_body_size") {
		location.client_max_body_size = Utils::parseSize(args[0]);
	} else if (directive == "cgi_ext") {
		if (args.size() < 2) {
			throw std::runtime_error(
			    "cgi_ext requires extension and interpreter path (e.g. cgi_ext "
			    ".py /usr/bin/python3;)");
		}
		location.cgi_handlers[args[0]] = args[1];
	} else {
		throw std::runtime_error(
		    "Unknown directive in location block: " + directive);
	}
}
