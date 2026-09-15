/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:10 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:55:42 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "config/Config.hpp"
#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include "parser/Lexer.hpp"
#include "utils/Logger.hpp"
#include <string>
#include <vector>

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
	void expect(const std::string&);
	void parseServer(Config&);
	LocationConfig parseLocation();
	void readDirective(std::string& name, std::vector<std::string>& args,
	    size_t& line, const std::string& indent);
	void parseServerDirective(ServerConfig& server);
	void parseLocationDirective(LocationConfig& loc);
};

#endif
