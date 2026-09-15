/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:32:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:46:36 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

enum TokenType { TOKEN_WORD, TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_SEMICOLON };

struct Token {
	TokenType type;
	std::string value;
	size_t line;

	Token(TokenType t, const std::string& val, size_t l);
};

class Lexer {
public:
	Lexer();
	Lexer(const Lexer& other);
	Lexer& operator=(const Lexer& other);
	~Lexer();

	std::vector<Token> tokenize(const std::string& content);
};

#endif
