/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 11:51:46 by sklaokli          #+#    #+#             */
/*   Updated: 2026/09/13 02:45:46 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include <iostream>
#include <string>

static bool parseArgs(int argc, char** argv, std::string& configPath) {
	configPath = "conf/default.conf";
	bool configSet = false;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--debug" || arg == "-d") {
			Logger::setLogLevel(DEBUG);
		} else if (arg[0] == '-') {
			Logger::error("Unknown option '" + arg + "'");
			return false;
		} else if (!configSet) {
			configPath = arg;
			configSet = true;
		} else {
			Logger::error("Too many arguments");
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv) {
	std::string configPath;
	if (!parseArgs(argc, argv, configPath)) return 1;

	Logger::info("Starting webserv...");
	try {
		Logger::info("Loading configuration...");
		Config config = Parser::parse(configPath);

		// Server webserv(config);
		// webserv.run();

	} catch (const std::exception& e) {
		Logger::error(e.what());
		return 1;
	} catch (...) {
		Logger::error("Unknown crash occurred");
		return 1;
	}

	Logger::info("Webserv stopped");
	return 0;
}
