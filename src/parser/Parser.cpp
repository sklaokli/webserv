/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:13:48 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 01:02:12 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Parser.hpp"
#include "utils/Utils.hpp"
#include <sstream>

static std::string formatLine(size_t line) {
	std::ostringstream oss;
	oss << "(L" << (line < 10 ? "0" : "") << line << ") ";
	return oss.str();
}

Parser::Parser(const std::string& filePath) : _filePath(filePath), _i(0) {
	std::string content = Utils::readFile(filePath);
	if (content.empty()) {
		throw std::runtime_error("Unable to read " + filePath);
	}

	Logger::debug("Read " + Utils::toString(content.length()) + " bytes from " +
	              filePath);

	Lexer lexer;
	_tokens = lexer.tokenize(content);
}

Parser::Parser(const Parser& other)
    : _filePath(other._filePath), _tokens(other._tokens), _i(other._i) {}

Parser& Parser::operator=(const Parser& other) {
	if (this != &other) {
		_filePath = other._filePath;
		_tokens = other._tokens;
		_i = other._i;
	}
	return *this;
}

Parser::~Parser() {}

Config Parser::execute() {
	Config config(_filePath);

	while (_i < _tokens.size()) {
		parseServer(config);
	}

	Logger::debug("Configuration parsed successfully!");
	return config;
}

void Parser::parseServer(Config& config) {
	size_t line = _tokens[_i].line;
	expect("server");
	Logger::debug(formatLine(line) + "Block: 'server'");

	expect("{");

	ServerConfig server;

	while (_i < _tokens.size() && _tokens[_i].value != "}") {
		if (_tokens[_i].value == "location") {
			LocationConfig loc = parseLocation();
			server.addLocation(loc);
		} else {
			parseServerDirective(server);
		}
	}

	expect("}");

	server.finalize();
	config.addServer(server);
}

LocationConfig Parser::parseLocation() {
	size_t line = _tokens[_i].line;
	expect("location");

	if (_i >= _tokens.size() || _tokens[_i].value == "{" ||
	    _tokens[_i].value == ";") {
		throw std::runtime_error(
		    "Missing location path on line " + Utils::toString(line));
	}

	std::string path = _tokens[_i].value;
	++_i;

	Logger::debug(
	    formatLine(line) + "  Block: 'location' path: \"" + path + "\"");

	expect("{");

	LocationConfig loc;
	loc.setPath(path);

	while (_i < _tokens.size() && _tokens[_i].value != "}") {
		parseLocationDirective(loc);
	}

	expect("}");

	return loc;
}

void Parser::readDirective(std::string& name, std::vector<std::string>& args,
    size_t& line, const std::string& indent) {
	line = _tokens[_i].line;
	name = _tokens[_i].value;
	++_i;

	if (_i >= _tokens.size() || _tokens[_i].value == ";" ||
	    _tokens[_i].value == "{" || _tokens[_i].value == "}") {
		throw std::runtime_error(
		    "Unexpected '" + (_i < _tokens.size() ? _tokens[_i].value : "EOF") +
		    "' on line " + Utils::toString(line));
	}

	args.clear();
	while (_i < _tokens.size() && _tokens[_i].value != ";" &&
	       _tokens[_i].value != "{" && _tokens[_i].value != "}") {
		args.push_back(_tokens[_i].value);
		++_i;
	}

	expect(";");

	std::string argsSummary = "[";
	for (size_t k = 0; k < args.size(); ++k) {
		if (k > 0) argsSummary += ", ";
		argsSummary += "\"" + args[k] + "\"";
	}
	argsSummary += "]";

	Logger::debug(formatLine(line) + indent + "Directive: '" + name + "' -> " +
	              argsSummary);
}

void Parser::parseServerDirective(ServerConfig& server) {
	std::string name;
	std::vector<std::string> args;
	size_t line;
	readDirective(name, args, line, "  ");

	if (name == "listen") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'listen' requires 1 argument on line " +
			    Utils::toString(line));
		}
		std::string arg = args[0];
		size_t colon = arg.find(':');
		if (colon != std::string::npos) {
			server.setHost(arg.substr(0, colon));
			int port = Utils::toInt(arg.substr(colon + 1));
			if (port <= 0 || port > 65535) {
				throw std::runtime_error(
				    "Invalid port on line " + Utils::toString(line));
			}
			server.setPort(port);
		} else {
			int port = Utils::toInt(arg);
			if (port <= 0 || port > 65535) {
				throw std::runtime_error(
				    "Invalid port on line " + Utils::toString(line));
			}
			server.setPort(port);
		}
	} else if (name == "host") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'host' requires 1 argument on line " +
			    Utils::toString(line));
		}
		server.setHost(args[0]);
	} else if (name == "server_name") {
		if (args.empty()) {
			throw std::runtime_error(
			    "Directive 'server_name' requires at least 1 argument on "
			    "line " +
			    Utils::toString(line));
		}
		for (size_t i = 0; i < args.size(); ++i) {
			server.addServerName(args[i]);
		}
	} else if (name == "client_max_body_size") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'client_max_body_size' requires 1 argument on "
			    "line " +
			    Utils::toString(line));
		}
		server.setClientMaxBodySize(Utils::parseSize(args[0]));
	} else if (name == "root") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'root' requires 1 argument on line " +
			    Utils::toString(line));
		}
		server.setRoot(args[0]);
	} else if (name == "index") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'index' requires 1 argument on line " +
			    Utils::toString(line));
		}
		server.setIndex(args[0]);
	} else if (name == "error_page") {
		if (args.size() < 2) {
			throw std::runtime_error(
			    "Directive 'error_page' requires at least 2 arguments on "
			    "line " +
			    Utils::toString(line));
		}
		std::string uri = args[args.size() - 1];
		for (size_t i = 0; i < args.size() - 1; ++i) {
			int code = Utils::toInt(args[i]);
			if (code < 300 || code > 599) {
				throw std::runtime_error("Invalid HTTP error code '" + args[i] +
				                         "' on line " + Utils::toString(line));
			}
			server.setErrorPage(code, uri);
		}
	} else {
		throw std::runtime_error("Unknown directive '" + name + "' on line " +
		                         Utils::toString(line));
	}
}

void Parser::parseLocationDirective(LocationConfig& loc) {
	std::string name;
	std::vector<std::string> args;
	size_t line;
	readDirective(name, args, line, "    ");

	if (name == "allow_methods") {
		if (args.empty()) {
			throw std::runtime_error(
			    "Directive 'allow_methods' requires at least 1 argument on "
			    "line " +
			    Utils::toString(line));
		}
		for (size_t i = 0; i < args.size(); ++i) {
			loc.addAllowedMethod(args[i]);
		}
	} else if (name == "root") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'root' requires 1 argument on line " +
			    Utils::toString(line));
		}
		loc.setRoot(args[0]);
	} else if (name == "index") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'index' requires 1 argument on line " +
			    Utils::toString(line));
		}
		loc.setIndex(args[0]);
	} else if (name == "autoindex") {
		if (args.size() != 1 || (args[0] != "on" && args[0] != "off")) {
			throw std::runtime_error(
			    "Directive 'autoindex' must be 'on' or 'off' on line " +
			    Utils::toString(line));
		}
		loc.setAutoindex(args[0] == "on");
	} else if (name == "return") {
		if (args.size() == 2) {
			int code = Utils::toInt(args[0]);
			loc.setRedirect(code, args[1]);
		} else if (args.size() == 1) {
			loc.setRedirect(302, args[0]);
		} else {
			throw std::runtime_error(
			    "Directive 'return' requires 1 or 2 arguments on line " +
			    Utils::toString(line));
		}
	} else if (name == "client_max_body_size") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'client_max_body_size' requires 1 argument on "
			    "line " +
			    Utils::toString(line));
		}
		loc.setClientMaxBodySize(Utils::parseSize(args[0]));
	} else if (name == "upload_enable") {
		if (args.size() != 1 || (args[0] != "on" && args[0] != "off")) {
			throw std::runtime_error(
			    "Directive 'upload_enable' must be 'on' or 'off' on line " +
			    Utils::toString(line));
		}
		loc.setUploadEnable(args[0] == "on");
	} else if (name == "upload_store") {
		if (args.size() != 1) {
			throw std::runtime_error(
			    "Directive 'upload_store' requires 1 argument on line " +
			    Utils::toString(line));
		}
		loc.setUploadStore(args[0]);
	} else if (name == "cgi_ext") {
		if (args.size() != 2) {
			throw std::runtime_error(
			    "Directive 'cgi_ext' requires extension and handler on line " +
			    Utils::toString(line));
		}
		loc.addCgiExt(args[0], args[1]);
	} else {
		throw std::runtime_error("Unknown directive '" + name +
		                         "' in location on line " +
		                         Utils::toString(line));
	}
}

void Parser::expect(const std::string& expected) {
	if (_i >= _tokens.size()) {
		throw std::runtime_error(
		    "Reached end of file while expecting '" + expected + "'");
	}

	if (_tokens[_i].value != expected) {
		std::string line = Utils::toString(_tokens[_i].line);
		std::string token = _tokens[_i].value;
		throw std::runtime_error("Expected '" + expected + "' on line " + line +
		                         ", found '" + token + "'");
	}

	++_i;
}
