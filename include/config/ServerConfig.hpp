/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:06:46 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "config/LocationConfig.hpp"
#include "parser/Lexer.hpp"
#include <map>
#include <string>
#include <vector>

class ServerConfig {
public:
	typedef std::map<int, std::string> ErrorPageMap;

	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	void applyDirective(const std::vector<Token>& tokens);
	LocationConfig& addLocation(const std::string& path);
	void finalize();

	void dump(size_t index = 0) const;

	const std::string& getHost() const;
	int getPort() const;
	const std::vector<std::string>& getServerNames() const;
	size_t getClientMaxBodySize() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	const ErrorPageMap& getErrorPages() const;
	const std::vector<LocationConfig>& getLocations() const;

	std::string getErrorPage(int code) const;
	const LocationConfig* findLocation(const std::string& uri) const;

private:
	void handleListen(const std::vector<Token>& tokens);
	void handleHost(const std::vector<Token>& tokens);
	void handleServerName(const std::vector<Token>& tokens);
	void handleClientMaxBodySize(const std::vector<Token>& tokens);
	void handleRoot(const std::vector<Token>& tokens);
	void handleIndex(const std::vector<Token>& tokens);
	void handleErrorPage(const std::vector<Token>& tokens);

	std::string _host;
	int _port;
	std::vector<std::string> _serverNames;
	size_t _clientMaxBodySize;
	std::string _root;
	std::string _index;
	ErrorPageMap _errorPages;
	std::vector<LocationConfig> _locations;
};

#endif
