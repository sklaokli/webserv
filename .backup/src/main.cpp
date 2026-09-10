#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

static Server* g_server = NULL;

static void handleSignal(int signum) {
	(void)signum;
	std::cout << "\n";
	Logger::info("Shutting down webserv...");
	if (g_server) {
		g_server->stop();
	}
}

int main(int argc, char** argv) {
	std::string configPath = "conf/default.conf";
	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration_file]"
		          << std::endl;
		return 1;
	} else if (argc == 2) {
		configPath = argv[1];
	}

	// Ignore SIGPIPE so writing to closed client/pipe doesn't crash the server!
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);

	try {
		Logger::info("Loading configuration: " + configPath);
		ConfigParser parser;
		Config config = parser.parse(configPath);

		Server server(config);
		g_server = &server;

		if (!server.init()) {
			Logger::error("Failed to initialize server");
			return 1;
		}

		server.run();
		g_server = NULL;

	} catch (const std::exception& e) {
		Logger::error("Fatal error: " + std::string(e.what()));
		return 1;
	}

	return 0;
}
