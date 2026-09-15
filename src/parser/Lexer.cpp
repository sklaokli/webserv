/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:32:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:55:42 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/Lexer.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <cctype>

Token::Token(TokenType t, const std::string& val, size_t l)
    : type(t), value(val), line(l) {}

Lexer::Lexer() {}

Lexer::Lexer(const Lexer& other) {
	(void)other;
}

Lexer& Lexer::operator=(const Lexer& other) {
	(void)other;
	return *this;
}

Lexer::~Lexer() {}

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
			if (!current.empty()) {
				tokens.push_back(Token(TOKEN_WORD, current, line));
				current.clear();
			}
			continue;
		}

		if (c == '{' || c == '}' || c == ';') {
			if (!current.empty()) {
				tokens.push_back(Token(TOKEN_WORD, current, line));
				current.clear();
			}
			TokenType type = (c == '{')   ? TOKEN_LBRACE
			                 : (c == '}') ? TOKEN_RBRACE
			                              : TOKEN_SEMICOLON;
			tokens.push_back(Token(type, std::string(1, c), line));
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(c))) {
			if (!current.empty()) {
				tokens.push_back(Token(TOKEN_WORD, current, line));
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
		tokens.push_back(Token(TOKEN_WORD, current, line));
	}

	Logger::debug("Tokenized " + Utils::toString(tokens.size()) +
	              " tokens across " + Utils::toString(line) + " lines");
	return tokens;
}
