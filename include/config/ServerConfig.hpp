/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/16 00:29:11 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "config/LocationConfig.hpp"
#include <map>
#include <string>
#include <vector>

class ServerConfig {
public:
	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	const std::string& getHost() const;
	int getPort() const;
	const std::vector<std::string>& getServerNames() const;
	size_t getClientMaxBodySize() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	const std::map<int, std::string>& getErrorPages() const;
	const std::vector<LocationConfig>& getLocations() const;

	void setHost(const std::string& host);
	void setPort(int port);
	void setServerNames(const std::vector<std::string>& names);
	void addServerName(const std::string& name);
	void setClientMaxBodySize(size_t size);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setErrorPage(int code, const std::string& uri);
	void addLocation(const LocationConfig& location);

	void finalize();
	std::string getErrorPage(int code) const;
	const LocationConfig* findLocation(const std::string& uri) const;

private:
	std::string _host;
	int _port;
	std::vector<std::string> _serverNames;
	size_t _clientMaxBodySize;
	std::string _root;
	std::string _index;
	std::map<int, std::string> _errorPages;
	std::vector<LocationConfig> _locations;
};

#endif
