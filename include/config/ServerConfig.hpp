/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 21:09:25 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "config/LocationConfig.hpp"
#include "parser/Lexer.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class ServerConfig {
public:
	typedef std::map<int, std::string> ErrorPageMap;

	ServerConfig();
	ServerConfig(const ServerConfig&);
	ServerConfig& operator=(const ServerConfig&);
	~ServerConfig();

	void applyDirective(const std::vector<Token>&);
	LocationConfig& addLocation(const std::string&);
	void finalize();

	void dump(size_t = 0) const;

	const std::string& getHost() const;
	int getPort() const;
	std::string getEndpoint() const;
	const std::vector<std::string>& getServerNames() const;
	size_t getClientMaxBodySize() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	const ErrorPageMap& getErrorPages() const;
	const std::vector<LocationConfig>& getLocations() const;

	std::string getErrorPage(int) const;
	const LocationConfig* findLocation(const std::string&) const;
	bool isDefault() const;
	void setDefault(bool);

private:
	static bool isSingleDirective(const std::string&);

	void handleListen(const std::vector<Token>&);
	void handleHost(const std::vector<Token>&);
	void handleServerName(const std::vector<Token>&);
	void handleClientMaxBodySize(const std::vector<Token>&);
	void handleRoot(const std::vector<Token>&);
	void handleIndex(const std::vector<Token>&);
	void handleErrorPage(const std::vector<Token>&);

	std::string _host;
	int _port;
	std::vector<std::string> _serverNames;
	size_t _clientMaxBodySize;
	std::string _root;
	std::string _index;
	ErrorPageMap _errorPages;
	std::vector<LocationConfig> _locations;
	bool _isDefault;
	std::set<std::string> _configuredDirectives;
};

#endif
