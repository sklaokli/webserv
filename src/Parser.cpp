/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:14 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 05:08:04 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>

Token::Token(const std::string& v, size_t l) : value(v), line(l) {}

Parser::Parser() {}

Parser::~Parser() {}

Config Parser::parse(const std::string& configPath) {
	std::string content = Utils::readFile(configPath);
	if (content.empty()) {
		throw std::runtime_error("Unable to read " + configPath);
	}

	Logger::debug("Read " + Utils::toString(content.length()) + " bytes from " +
	              configPath);

	std::vector<Token> tokens = tokenize(content);

	Config config;

	return config;
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

	// for (size_t k = 0; k < tokens.size(); ++k) {
	// 	Logger::debug("Token #" + Utils::toString(tokens[k].id) + " - " +
	// 	              tokens[k].value);
	// }

	Logger::debug("Tokenized " + Utils::toString(tokens.size()) +
	              " tokens across " + Utils::toString(line) + " lines");
	return tokens;
}

void Parser::expect(
    const std::vector<Token>& tokens, size_t& i, const std::string& expected) {
	if (i >= tokens.size()) {
		throw std::runtime_error(
		    "Reached end of file while expecting '" + expected + "'");
	}

	if (tokens[i].value != expected) {
		std::string line = Utils::toString(tokens[i].line);
		std::string token = tokens[i].value;
		throw std::runtime_error("Expected '" + expected + "' on line " + line +
		                         ", found '" + token + "'");
	}

	++i;
}

Parser::Parser(const Parser& other) {
	(void)other;
}

Parser& Parser::operator=(const Parser& other) {
	(void)other;
	return *this;
}
