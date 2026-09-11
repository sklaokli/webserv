/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 11:51:46 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/11 14:59:26 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
	std::string configPath;

	if (argc == 1) {
		configPath = "conf/default.conf";
	} else if (argc == 2) {
		configPath = argv[1];
	} else {
		std::cerr << "Usage: " << argv[0] << " [configPath]" << std::endl;
		return 1;
	}

	try {
		Logger::info("Loading configuration: " + configPath);
	} catch (const std::exception& e) {
		Logger::error("Fatal: " + std::string(e.what()));
		return 1;
	}

	return 0;
}
