/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 01:22:10 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:29:01 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "config/Config.hpp"
#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include "parser/Lexer.hpp"
#include <set>
#include <string>
#include <vector>

struct Directive {
	std::string name;
	std::vector<std::string> args;
	size_t line;

	Directive();
	Directive(const std::string&, size_t);
};

class Parser {
public:
	Parser();
	explicit Parser(const std::string&);
	Parser(const Parser&);
	Parser& operator=(const Parser&);
	~Parser();

	bool parseArgs(int, char**);
	Config parseConfig();
	Config parseConfig(const std::string&);

	const std::string& getFilePath() const;

private:
	std::string _filePath;
	std::vector<Token> _tokens;
	std::vector<Token>::const_iterator _it;
	std::vector<Token>::const_iterator _end;

	void parse(const std::string&, Config&);
	bool isDone() const;
	void expect(const std::string&);
	Directive readDirective();

	void parseServer(Config&);
	void parseServerDirective(ServerConfig&, std::set<std::string>&);
	void parseListen(const Directive&, ServerConfig&);
	void parseHost(const Directive&, ServerConfig&);
	void parseServerName(const Directive&, ServerConfig&);
	void parseErrorPage(const Directive&, ServerConfig&);

	void parseLocation(std::vector<LocationConfig>&);
	void parseLocationDirective(LocationConfig&, std::set<std::string>&);
	void parseAllowMethods(const Directive&, LocationConfig&);
	void parseAutoindex(const Directive&, LocationConfig&);
	void parseReturn(const Directive&, LocationConfig&);
	void parseUploadEnable(const Directive&, LocationConfig&);
	void parseUploadStore(const Directive&, LocationConfig&);
	void parseCgiExt(const Directive&, LocationConfig&);

	static std::string parseRoot(const Directive&);
	static std::string parseIndex(const Directive&);
	static size_t parseClientMaxBodySize(const Directive&);
	static size_t parseSize(const std::string&);

	static void inheritLocation(LocationConfig&, const ServerConfig&);
	static void validate(Config&);
	static void checkSingle(const std::string&, size_t, std::set<std::string>&);

	static bool isValidHost(const std::string&);
	static bool isValidPort(int);
	static bool isValidMethod(const std::string&);
	static bool isValidErrorCode(int);
	static bool isValidRedirectCode(int);

	static void assertArgs(const Directive&, size_t);
	static void assertArgs(const Directive&, size_t, size_t);
	static void assertMinArgs(const Directive&, size_t);
};

#endif
