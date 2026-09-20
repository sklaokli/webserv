/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:32:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/20 19:45:40 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

struct Token {
	std::string value;
	size_t line;

	Token();
	Token(const std::string&, size_t);
};

class Lexer {
public:
	static std::vector<Token> tokenize(const std::string&);

private:
	Lexer();
	Lexer(const Lexer&);
	Lexer& operator=(const Lexer&);
	~Lexer();
};

#endif
