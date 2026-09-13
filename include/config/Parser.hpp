/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:10 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 17:16:41 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include <string>
#include <vector>

struct Token {
	std::string value;
	size_t line;
	Token(const std::string&, size_t);
};

class Parser {
public:
	Parser(const std::string& filePath);
	Parser(const Parser& other);
	Parser& operator=(const Parser& other);
	~Parser();

	Config execute();

private:
	std::string _filePath;
	std::vector<Token> _tokens;
	size_t _i;

	Parser();
	std::vector<Token> tokenize(const std::string&);
	void expect(const std::string&);
	void parseServer(Config&);
	void parseLocation();
	void parseDirective(const std::string&);
};

#endif
