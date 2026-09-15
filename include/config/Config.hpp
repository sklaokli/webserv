/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 02:22:53 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/15 20:33:33 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

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

private:
	std::string _path;
};

#endif
