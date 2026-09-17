/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:10 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:04:19 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include "parser/Lexer.hpp"
#include <string>
#include <vector>

class Parser {
public:
	Parser(const std::vector<Token>& tokens);
	Parser(const Parser& other);
	Parser& operator=(const Parser& other);
	~Parser();

	void parseServer(
	    ServerConfig& server, std::vector<Token>::const_iterator& it);

private:
	std::vector<Token>::const_iterator _it;
	std::vector<Token>::const_iterator _end;

	ServerConfig* _curServer;
	LocationConfig* _curLoc;
	std::vector<Token> _directiveTokens;

	Parser();
	void expect(const std::string& expected);
	void parseLocation();
	void readDirective();

	void parseServerDirective();
	void parseLocationDirective();
};

#endif
