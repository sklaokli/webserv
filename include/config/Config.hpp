/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:22:53 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 21:39:31 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "config/ServerConfig.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Config {
public:
	struct SizeUnits {
		static const size_t KB = 1024;
		static const size_t MB = 1024 * 1024;
		static const size_t GB = 1024 * 1024 * 1024;
	};

	struct IpV4 {
		static const int MIN = 0;
		static const int MAX = 255;
	};

	Config();
	explicit Config(const std::string&);
	Config(const Config&);
	Config& operator=(const Config&);
	~Config();

	void dump() const;

	const std::string& getPath() const;
	void setPath(const std::string&);

	const std::vector<ServerConfig>& getServers() const;
	std::vector<ServerConfig>& getServers();
	void addServer(const ServerConfig&);

private:
	std::string _path;
	std::vector<ServerConfig> _servers;
};

#endif
