/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:22:53 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 00:12:31 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "config/ServerConfig.hpp"
#include <string>
#include <vector>

class Config {
public:
	Config();
	Config(const std::string& configPath);
	Config(const Config& other);
	Config& operator=(const Config& other);
	~Config();

	void parse();
	const std::string& getPath() const;
	const std::vector<ServerConfig>& getServers() const;
	void dump() const;

private:
	ServerConfig& addServer();
	void validate();

	std::string _path;
	std::vector<ServerConfig> _servers;
};

#endif
