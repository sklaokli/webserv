/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:10 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 05:06:06 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "Config.hpp"
#include "Logger.hpp"
#include <string>
#include <vector>

struct Token {
	std::string value;
	size_t line;
	Token(const std::string&, size_t);
};

class Parser {
public:
	Parser();
	~Parser();

	static Config parse(const std::string&);

private:
	Parser(const Parser&);
	Parser& operator=(const Parser&);

	static std::vector<Token> tokenize(const std::string&);
	static void expect(const std::vector<Token>& tokens, size_t& i,
	    const std::string& expected);
	static void parseServer(
	    const std::vector<Token>& tokens, size_t& i, Config& config);
	static void parseLocation(const std::vector<Token>& tokens, size_t& i);
	static void parseDirective(
	    const std::vector<Token>& tokens, size_t& i, const std::string& indent);
};

#endif
