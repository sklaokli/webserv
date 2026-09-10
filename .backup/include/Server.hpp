#ifndef SERVER_HPP
#define SERVER_HPP

#include "CgiHandler.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Poller.hpp"
#include <map>
#include <vector>

class Server {
public:
	Server(const Config& config);
	~Server();

	bool init();
	void run();
	void stop();

private:
	Config m_config;
	Poller m_poller;
	bool m_running;

	std::map<int, ServerConfig> m_listeners;  // listenFd -> ServerConfig
	std::map<int, Client*> m_clients;         // clientFd -> Client*

	std::map<int, CgiHandler*> m_cgiInPipes;   // inPipeFd -> CgiHandler*
	std::map<int, CgiHandler*> m_cgiOutPipes;  // outPipeFd -> CgiHandler*
	std::map<int, CgiHandler*> m_clientCgis;   // clientFd -> CgiHandler*

	bool createListeningSocket(
	    const std::string& host, int port, const ServerConfig& serverConfig);
	void handleNewConnection(int listenFd);
	void handleClientRead(int clientFd);
	void handleClientWrite(int clientFd);
	void handleCgiPipeWrite(int inPipeFd);
	void handleCgiPipeRead(int outPipeFd);

	void processClientRequest(Client* client);
	void checkTimeouts();
	void closeClient(int clientFd);
	void closeCgi(CgiHandler* cgi);
};

#endif
