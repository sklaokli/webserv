/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:22:53 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/18 19:00:29 by sklaokli         ###   ########.fr       */
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
	explicit Config(const std::string&);
	Config(const Config&);
	Config& operator=(const Config&);
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
