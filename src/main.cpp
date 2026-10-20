/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 11:51:46 by sklaokli          #+#    #+#             */
/*   Updated: 2026/10/20 20:30:00 by sklaokli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Config.hpp"
#include "parser/Parser.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
	Parser parser;
	if (!parser.parseArgs(argc, argv)) return 1;

	Logger::info("Starting webserv...");
	try {
		Logger::info("Loading configuration...");
		Config config = parser.parseConfig();

		// Server webserv(config);
		// webserv.run();

	} catch (const std::exception& e) {
		Logger::error(e.what());
		return 1;
	} catch (...) {
		Logger::error("Unknown crash occurred");
		return 1;
	}

	Logger::info("Stopping webserv...");
	return 0;
}
