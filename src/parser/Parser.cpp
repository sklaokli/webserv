/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 15:13:48 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:19:18 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Parser.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <sstream>
#include <stdexcept>

static std::string formatLine(size_t line) {
	std::ostringstream oss;
	oss << "(L" << (line < 10 ? "0" : "") << line << ") ";
	return oss.str();
}

Parser::Parser(const std::vector<Token>& tokens)
    : _it(tokens.begin())
    , _end(tokens.end())
    , _curServer(NULL)
    , _curLoc(NULL)
    , _directiveTokens() {}

Parser::Parser(const Parser& other)
    : _it(other._it)
    , _end(other._end)
    , _curServer(NULL)
    , _curLoc(NULL)
    , _directiveTokens(other._directiveTokens) {}

Parser& Parser::operator=(const Parser& other) {
	if (this != &other) {
		_it = other._it;
		_end = other._end;
		_curServer = NULL;
		_curLoc = NULL;
		_directiveTokens = other._directiveTokens;
	}
	return *this;
}

Parser::~Parser() {}

void Parser::parseServer(
    ServerConfig& server, std::vector<Token>::const_iterator& it) {
	_it = it;
	_curServer = &server;

	if (_it == _end) {
		throw std::runtime_error(
		    "Reached end of file while expecting 'server'");
	}
	size_t line = _it->line;
	expect("server");
	Logger::debug(formatLine(line) + "Block: 'server'");

	expect("{");

	while (_it != _end && _it->value != "}") {
		if (_it->value == "location") {
			parseLocation();
		} else {
			parseServerDirective();
		}
	}

	expect("}");

	_curServer->finalize();
	_curServer = NULL;
	it = _it;
}

void Parser::parseLocation() {
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
	++_it;

	Logger::debug(
	    formatLine(line) + "  Block: 'location' path: \"" + path + "\"");

	expect("{");

	_curLoc = &_curServer->addLocation(path);

	while (_it != _end && _it->value != "}") {
		parseLocationDirective();
	}

	expect("}");

	_curLoc = NULL;
}

void Parser::readDirective() {
	_directiveTokens.clear();
	if (_it == _end) {
		throw std::runtime_error("Unexpected EOF while reading directive");
	}
	size_t line = _it->line;
	std::string name = _it->value;
	_directiveTokens.push_back(*_it);
	++_it;

	if (_it == _end || _it->value == ";" || _it->value == "{" ||
	    _it->value == "}") {
		throw std::runtime_error("Unexpected '" +
		                         (_it != _end ? _it->value : "EOF") +
		                         "' on line " + Utils::toString(line));
	}

	while (_it != _end && _it->value != ";" && _it->value != "{" &&
	       _it->value != "}") {
		_directiveTokens.push_back(*_it);
		++_it;
	}

	expect(";");

	std::string argsSummary = "[";
	for (std::vector<Token>::const_iterator it = _directiveTokens.begin() + 1;
	     it != _directiveTokens.end(); ++it) {
		if (it != _directiveTokens.begin() + 1) argsSummary += ", ";
		argsSummary += "\"" + it->value + "\"";
	}
	argsSummary += "]";

	std::string indent = (_curLoc != NULL ? "    " : "  ");
	Logger::debug(formatLine(line) + indent + "Directive: '" + name + "' -> " +
	              argsSummary);
}

void Parser::parseServerDirective() {
	readDirective();
	_curServer->applyDirective(_directiveTokens);
}

void Parser::parseLocationDirective() {
	readDirective();
	_curLoc->applyDirective(_directiveTokens);
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
