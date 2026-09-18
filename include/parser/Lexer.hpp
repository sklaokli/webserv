/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:32:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 19:00:29 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

struct Token {
	std::string value;
	size_t line;

	Token(const std::string&, size_t);
};

class Lexer {
public:
	Lexer();
	Lexer(const Lexer&);
	Lexer& operator=(const Lexer&);
	~Lexer();

	static std::vector<Token> tokenize(const std::string&);
};

#endif
