/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:13:48 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 17:16:37 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Parser.hpp"
#include "utils/Utils.hpp"
#include <sstream>

static std::string formatLine(size_t line) {
	std::ostringstream oss;
	oss << "(L" << (line < 10 ? "0" : "") << line << ") ";
	return oss.str();
}

Token::Token(const std::string& v, size_t l) : value(v), line(l) {}

Parser::Parser(const std::string& filePath) : _filePath(filePath), _i(0) {
	std::string content = Utils::readFile(filePath);
	if (content.empty()) {
		throw std::runtime_error("Unable to read " + filePath);
	}

	Logger::debug("Read " + Utils::toString(content.length()) + " bytes from " +
	              filePath);

	_tokens = tokenize(content);
}

Parser::~Parser() {}

Config Parser::execute() {
	Config config(_filePath);

	while (_i < _tokens.size()) {
		parseServer(config);
	}

	Logger::info("Configuration parsed successfully!");
	return config;
}

void Parser::parseServer(Config& config) {
	(void)config;

	size_t line = _tokens[_i].line;
	expect("server");
	Logger::debug(formatLine(line) + "Block: 'server'");

	expect("{");

	while (_i < _tokens.size() && _tokens[_i].value != "}") {
		if (_tokens[_i].value == "location") {
			parseLocation();
		} else {
			parseDirective("  ");
		}
	}

	expect("}");
}

void Parser::parseLocation() {
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

	while (_i < _tokens.size() && _tokens[_i].value != "}") {
		parseDirective("    ");
	}

	expect("}");
}

void Parser::parseDirective(const std::string& indent) {
	std::string directive = _tokens[_i].value;
	size_t line = _tokens[_i].line;
	++_i;

	if (directive == ";" || directive == "{" || directive == "}") {
		throw std::runtime_error(
		    "Unexpected '" + directive + "' on line " + Utils::toString(line));
	}

	std::vector<std::string> args;
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

	Logger::debug(formatLine(line) + indent + "Directive: '" + directive +
	              "' -> " + argsSummary);
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

std::vector<Token> Parser::tokenize(const std::string& content) {
	std::vector<Token> tokens;
	std::string current;
	bool inComment = false;
	size_t line = 1;

	for (size_t i = 0; i < content.length(); ++i) {
		char c = content[i];

		if (inComment) {
			if (c == '\n') {
				inComment = false;
				++line;
			}
			continue;
		}

		if (c == '#') {
			inComment = true;
			if (!current.empty()) {
				tokens.push_back(Token(current, line));
				current.clear();
			}
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!current.empty()) {
				tokens.push_back(Token(current, line));
				current.clear();
			}
			tokens.push_back(Token(std::string(1, c), line));
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(c))) {
			if (!current.empty()) {
				tokens.push_back(Token(current, line));
				current.clear();
			}
			if (c == '\n') {
				++line;
			}
			continue;
		}

		current += c;
	}

	if (!current.empty()) {
		tokens.push_back(Token(current, line));
	}

	Logger::debug("Tokenized " + Utils::toString(tokens.size()) +
	              " tokens across " + Utils::toString(line) + " lines");
	return tokens;
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
