/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:20:00 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:44:00 by sklaokli         ###   ########.fr       */
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
	typedef std::map<int, std::string> ErrorPageMap;

	struct Ports {
		static const int MIN = 1;
		static const int MAX = 65535;
	};

	struct StatusCodes {
		static const int MIN = 300;
		static const int MAX = 599;
	};

	ServerConfig();
	ServerConfig(const ServerConfig&);
	ServerConfig& operator=(const ServerConfig&);
	~ServerConfig();

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
	std::vector<LocationConfig>& getLocations();
	bool isDefault() const;

	void setHost(const std::string&);
	void setPort(int);
	void setServerNames(const std::vector<std::string>&);
	void addServerName(const std::string&);
	void setClientMaxBodySize(size_t);
	void setRoot(const std::string&);
	void setIndex(const std::string&);
	void setErrorPages(const ErrorPageMap&);
	void addErrorPage(int, const std::string&);
	void addLocation(const LocationConfig&);
	void setDefault(bool);

	std::string getErrorPage(int) const;
	const LocationConfig* findLocation(const std::string&) const;

private:
	std::string _host;
	int _port;
	std::vector<std::string> _serverNames;
	size_t _clientMaxBodySize;
	std::string _root;
	std::string _index;
	ErrorPageMap _errorPages;
	std::vector<LocationConfig> _locations;
	bool _isDefault;
};

#endif
