/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:32:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:05:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Lexer.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <cctype>

Token::Token() : value(""), line(0) {}

Token::Token(const std::string& val, size_t l) : value(val), line(l) {}

Lexer::Lexer() {}

Lexer::Lexer(const Lexer& other) {
	(void)other;
}

Lexer& Lexer::operator=(const Lexer& other) {
	(void)other;
	return *this;
}

Lexer::~Lexer() {}

static void flushToken(
    std::vector<Token>& tokens, std::string& current, size_t line) {
	if (!current.empty()) {
		tokens.push_back(Token(current, line));
		current.clear();
	}
}

std::vector<Token> Lexer::tokenize(const std::string& content) {
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
			flushToken(tokens, current, line);
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			flushToken(tokens, current, line);
			tokens.push_back(Token(std::string(1, c), line));
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(c))) {
			flushToken(tokens, current, line);
			if (c == '\n') {
				++line;
			}
			continue;
		}

		current += c;
	}

	flushToken(tokens, current, line);

	Logger::debug(
	    "Tokenized " + Utils::toString(tokens.size()) + " tokens across " +
	    Utils::toString(line) + " lines");
	return tokens;
}
