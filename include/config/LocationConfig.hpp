/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:06:43 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "parser/Lexer.hpp"
#include <map>
#include <string>
#include <vector>

class ServerConfig;

class LocationConfig {
public:
	typedef std::map<std::string, std::string> CgiMap;

	LocationConfig();
	LocationConfig(const std::string& path);
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	void applyDirective(const std::vector<Token>& tokens);
	void inherit(const ServerConfig& server);

	void dump(size_t serverBodySize = 0) const;

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

	bool isMethodAllowed(const std::string& method) const;
	bool hasRedirect() const;
	bool hasCgi(const std::string& ext) const;
	std::string getCgiHandler(const std::string& ext) const;

private:
	void handleAllowMethods(const std::vector<Token>& tokens);
	void handleRoot(const std::vector<Token>& tokens);
	void handleIndex(const std::vector<Token>& tokens);
	void handleAutoindex(const std::vector<Token>& tokens);
	void handleReturn(const std::vector<Token>& tokens);
	void handleClientMaxBodySize(const std::vector<Token>& tokens);
	void handleUploadEnable(const std::vector<Token>& tokens);
	void handleUploadStore(const std::vector<Token>& tokens);
	void handleCgiExt(const std::vector<Token>& tokens);

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
};

#endif
