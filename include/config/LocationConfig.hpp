/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 20:42:10 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "parser/Lexer.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class ServerConfig;

class LocationConfig {
public:
	typedef std::map<std::string, std::string> CgiMap;

	LocationConfig();
	explicit LocationConfig(const std::string&);
	LocationConfig(const LocationConfig&);
	LocationConfig& operator=(const LocationConfig&);
	~LocationConfig();

	void applyDirective(const std::vector<Token>&);
	void inherit(const ServerConfig&);

	void dump(size_t = 0) const;

	const std::string& getPath() const;
	const std::vector<std::string>& getAllowedMethods() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	bool getAutoindex() const;
	int getRedirectCode() const;
	const std::string& getRedirectUrl() const;
	bool getUploadEnable() const;
	const std::string& getUploadStore() const;
	const CgiMap& getCgiExt() const;
	size_t getClientMaxBodySize() const;

	bool isMethodAllowed(const std::string&) const;
	bool hasRedirect() const;
	bool hasCgi(const std::string&) const;
	std::string getCgiHandler(const std::string&) const;

private:
	static bool isSingleDirective(const std::string&);

	void handleAllowMethods(const std::vector<Token>&);
	void handleRoot(const std::vector<Token>&);
	void handleIndex(const std::vector<Token>&);
	void handleAutoindex(const std::vector<Token>&);
	void handleReturn(const std::vector<Token>&);
	void handleClientMaxBodySize(const std::vector<Token>&);
	void handleUploadEnable(const std::vector<Token>&);
	void handleUploadStore(const std::vector<Token>&);
	void handleCgiExt(const std::vector<Token>&);

	std::string _path;
	std::vector<std::string> _allowedMethods;
	std::string _root;
	std::string _index;
	bool _autoindex;
	int _redirectCode;
	std::string _redirectUrl;
	bool _uploadEnable;
	std::string _uploadStore;
	CgiMap _cgiExt;
	size_t _clientMaxBodySize;
	std::set<std::string> _configuredDirectives;
};

#endif
