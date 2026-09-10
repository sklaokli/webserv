#include "Server.hpp"
#include "Logger.hpp"
#include "Router.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

Server::Server(const Config& config) : m_config(config), m_running(false) {}

Server::~Server() {
	stop();
}

bool Server::createListeningSocket(
    const std::string& host, int port, const ServerConfig& serverConfig) {
	int listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd < 0) {
		Logger::error(
		    "socket() failed for " + host + ":" + Utils::toString(port));
		return false;
	}

	int opt = 1;
	if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		Logger::error("setsockopt() SO_REUSEADDR failed");
		close(listenFd);
		return false;
	}

	// Set non-blocking and close-on-exec
	if (fcntl(listenFd, F_SETFL, O_NONBLOCK) < 0 ||
	    fcntl(listenFd, F_SETFD, FD_CLOEXEC) < 0) {
		Logger::error("fcntl() failed for listening socket");
		close(listenFd);
		return false;
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (host == "0.0.0.0" || host.empty()) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
			Logger::error("Invalid listen address: " + host);
			close(listenFd);
			return false;
		}
	}

	if (bind(listenFd, reinterpret_cast<struct sockaddr*>(&addr),
	        sizeof(addr)) < 0) {
		Logger::error("bind() failed on " + host + ":" + Utils::toString(port) +
		              ": " + strerror(errno));
		close(listenFd);
		return false;
	}

	if (listen(listenFd, SOMAXCONN) < 0) {
		Logger::error(
		    "listen() failed on " + host + ":" + Utils::toString(port));
		close(listenFd);
		return false;
	}

	if (!m_poller.addFd(listenFd, EVENT_READ)) {
		Logger::error("Failed to add listening socket to poller");
		close(listenFd);
		return false;
	}

	m_listeners[listenFd] = serverConfig;
	Logger::info("Listening on " + host + ":" + Utils::toString(port) +
	             " (fd: " + Utils::toString(listenFd) + ")");
	return true;
}

bool Server::init() {
	if (!m_poller.init()) {
		return false;
	}

	const std::vector<ServerConfig>& servers = m_config.getServers();
	std::vector<std::pair<std::string, int> > boundEndpoints;

	for (size_t i = 0; i < servers.size(); ++i) {
		const ServerConfig& s = servers[i];
		bool alreadyBound = false;

		for (size_t j = 0; j < boundEndpoints.size(); ++j) {
			if (boundEndpoints[j].second == s.port &&
			    (boundEndpoints[j].first == s.host || s.host == "0.0.0.0" ||
			        boundEndpoints[j].first == "0.0.0.0")) {
				alreadyBound = true;
				break;
			}
		}

		if (!alreadyBound) {
			if (createListeningSocket(s.host, s.port, s)) {
				boundEndpoints.push_back(std::make_pair(s.host, s.port));
			} else {
				Logger::error("Failed to start listener on port " +
				              Utils::toString(s.port));
				return false;
			}
		}
	}

	if (m_listeners.empty()) {
		Logger::error("No listening sockets could be created");
		return false;
	}

	return true;
}

void Server::run() {
	m_running = true;
	Logger::info("Webserv started. Waiting for connections...");

	std::vector<PollEvent> triggeredEvents;

	while (m_running) {
		int numEvents =
		    m_poller.pollEvents(triggeredEvents, 1000);  // 1-second timeout
		if (numEvents < 0) {
			if (errno == EINTR) continue;
			Logger::error("pollEvents failed: " + std::string(strerror(errno)));
			break;
		}

		for (size_t i = 0; i < triggeredEvents.size(); ++i) {
			int fd = triggeredEvents[i].fd;
			int events = triggeredEvents[i].events;

			if (m_listeners.find(fd) != m_listeners.end()) {
				if (events & EVENT_READ) {
					handleNewConnection(fd);
				}
			} else if (m_cgiInPipes.find(fd) != m_cgiInPipes.end()) {
				if (events & (EVENT_WRITE | EVENT_ERROR)) {
					handleCgiPipeWrite(fd);
				}
			} else if (m_cgiOutPipes.find(fd) != m_cgiOutPipes.end()) {
				if (events & (EVENT_READ | EVENT_ERROR)) {
					handleCgiPipeRead(fd);
				}
			} else if (m_clients.find(fd) != m_clients.end()) {
				if (events & EVENT_ERROR) {
					closeClient(fd);
				} else {
					if (events & EVENT_READ) {
						handleClientRead(fd);
					}
					if (m_clients.find(fd) != m_clients.end() &&
					    (events & EVENT_WRITE)) {
						handleClientWrite(fd);
					}
				}
			}
		}

		checkTimeouts();
	}
}

void Server::stop() {
	if (!m_running && m_listeners.empty() && m_clients.empty()) return;

	m_running = false;

	// Clean up CGI handlers
	for (std::map<int, CgiHandler*>::iterator it = m_clientCgis.begin();
	     it != m_clientCgis.end(); ++it) {
		delete it->second;
	}
	m_clientCgis.clear();
	m_cgiInPipes.clear();
	m_cgiOutPipes.clear();

	// Close clients
	for (std::map<int, Client*>::iterator it = m_clients.begin();
	     it != m_clients.end(); ++it) {
		m_poller.removeFd(it->first);
		delete it->second;
	}
	m_clients.clear();

	// Close listeners
	for (std::map<int, ServerConfig>::iterator it = m_listeners.begin();
	     it != m_listeners.end(); ++it) {
		m_poller.removeFd(it->first);
		close(it->first);
	}
	m_listeners.clear();

	Logger::info("Webserv stopped successfully.");
}

void Server::handleNewConnection(int listenFd) {
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientFd = accept(
	    listenFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
	if (clientFd < 0) {
		return;
	}

	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	fcntl(clientFd, F_SETFD, FD_CLOEXEC);

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
	int clientPort = ntohs(clientAddr.sin_port);

	const ServerConfig* serverConfig = &m_listeners[listenFd];
	Client* client = new Client(clientFd, ipStr, clientPort, serverConfig);
	m_clients[clientFd] = client;

	// Register for EVENT_READ; EVENT_WRITE will be enabled dynamically when a
	// response is ready
	m_poller.addFd(clientFd, EVENT_READ);
}

void Server::handleClientRead(int clientFd) {
	std::map<int, Client*>::iterator it = m_clients.find(clientFd);
	if (it == m_clients.end()) return;

	Client* client = it->second;
	char buffer[8192];
	ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytesRead <= 0) {
		closeClient(clientFd);
		return;
	}

	client->updateActivity();

	size_t maxBody = client->getServerConfig()
	                     ? client->getServerConfig()->client_max_body_size
	                     : 0;
	bool complete = client->getRequest().parseChunk(
	    std::string(buffer, bytesRead), maxBody);

	if (complete) {
		// Refine ServerConfig matching by Host header
		std::string hostHeader = client->getRequest().getHeader("host");
		if (!hostHeader.empty()) {
			size_t colonPos = hostHeader.find(':');
			std::string serverName = (colonPos != std::string::npos)
			                             ? hostHeader.substr(0, colonPos)
			                             : hostHeader;
			const ServerConfig* matchedServer =
			    m_config.findServer(client->getServerConfig()->host,
			        client->getServerConfig()->port, serverName);
			if (matchedServer) {
				client->setServerConfig(matchedServer);
			}
		}

		processClientRequest(client);
	}
}

void Server::processClientRequest(Client* client) {
	const ServerConfig* server = client->getServerConfig();
	const LocationConfig* loc =
	    server ? server->findLocation(client->getRequest().getPath()) : NULL;

	std::string scriptPath;
	std::string interpreter;

	if (Router::isCgiRequest(
	        client->getRequest(), loc, scriptPath, interpreter)) {
		CgiHandler* cgi = new CgiHandler();
		if (cgi->start(client->getRequest(), *loc, *server, scriptPath,
		        interpreter, client->getFd(), client->getIp())) {
			if (cgi->getInPipeFd() != -1) {
				m_cgiInPipes[cgi->getInPipeFd()] = cgi;
				m_poller.addFd(cgi->getInPipeFd(), EVENT_WRITE);
			}
			if (cgi->getOutPipeFd() != -1) {
				m_cgiOutPipes[cgi->getOutPipeFd()] = cgi;
				m_poller.addFd(cgi->getOutPipeFd(), EVENT_READ);
			}
			m_clientCgis[client->getFd()] = cgi;
			client->setState(CLIENT_CGI_PROCESSING);
			return;
		} else {
			delete cgi;
			HttpResponse errRes = Router::buildErrorResponse(
			    500, *server, "Failed to Execute CGI");
			client->setResponse(errRes, false);
			m_poller.modifyFd(client->getFd(), EVENT_READ | EVENT_WRITE);
			return;
		}
	}

	HttpResponse response =
	    Router::handleRequest(client->getRequest(), *server);
	bool keepAlive = client->getRequest().isKeepAlive();
	if (client->getRequest().hasError() || response.getStatusCode() >= 400) {
		keepAlive = false;
	}
	client->setResponse(response, keepAlive);
	m_poller.modifyFd(client->getFd(), EVENT_READ | EVENT_WRITE);
}

void Server::handleClientWrite(int clientFd) {
	std::map<int, Client*>::iterator it = m_clients.find(clientFd);
	if (it == m_clients.end()) return;

	Client* client = it->second;
	if (!client->hasDataToSend()) return;

	const std::string& buffer = client->getWriteBuffer();
	ssize_t bytesSent = send(clientFd, buffer.c_str(), buffer.length(), 0);

	if (bytesSent <= 0) {
		closeClient(clientFd);
		return;
	}

	client->consumeWriteBuffer(bytesSent);
	client->updateActivity();

	if (!client->hasDataToSend()) {
		if (client->shouldClose()) {
			closeClient(clientFd);
		} else {
			client->resetForNextRequest();
			m_poller.modifyFd(clientFd, EVENT_READ);
		}
	}
}

void Server::handleCgiPipeWrite(int inPipeFd) {
	std::map<int, CgiHandler*>::iterator it = m_cgiInPipes.find(inPipeFd);
	if (it == m_cgiInPipes.end()) return;

	CgiHandler* cgi = it->second;
	cgi->writeInputChunk();

	if (!cgi->hasInputToWrite()) {
		m_poller.removeFd(inPipeFd);
		m_cgiInPipes.erase(it);
	}
}

void Server::handleCgiPipeRead(int outPipeFd) {
	std::map<int, CgiHandler*>::iterator it = m_cgiOutPipes.find(outPipeFd);
	if (it == m_cgiOutPipes.end()) return;

	CgiHandler* cgi = it->second;
	cgi->readOutputChunk();

	if (cgi->isOutputComplete()) {
		m_poller.removeFd(outPipeFd);
		m_cgiOutPipes.erase(it);

		HttpResponse res = cgi->buildResponse();
		int clientFd = cgi->getClientFd();

		std::map<int, Client*>::iterator clientIt = m_clients.find(clientFd);
		if (clientIt != m_clients.end()) {
			bool keepAlive = clientIt->second->getRequest().isKeepAlive();
			if (res.getStatusCode() >= 400) {
				keepAlive = false;
			}
			clientIt->second->setResponse(res, keepAlive);
			m_poller.modifyFd(clientFd, EVENT_READ | EVENT_WRITE);
		}

		closeCgi(cgi);
	}
}

void Server::closeCgi(CgiHandler* cgi) {
	if (!cgi) return;

	if (cgi->getInPipeFd() != -1) {
		m_poller.removeFd(cgi->getInPipeFd());
		m_cgiInPipes.erase(cgi->getInPipeFd());
	}
	if (cgi->getOutPipeFd() != -1) {
		m_poller.removeFd(cgi->getOutPipeFd());
		m_cgiOutPipes.erase(cgi->getOutPipeFd());
	}
	m_clientCgis.erase(cgi->getClientFd());

	cgi->cleanup();
	delete cgi;
}

void Server::closeClient(int clientFd) {
	std::map<int, Client*>::iterator it = m_clients.find(clientFd);
	if (it == m_clients.end()) return;

	std::map<int, CgiHandler*>::iterator cgiIt = m_clientCgis.find(clientFd);
	if (cgiIt != m_clientCgis.end()) {
		closeCgi(cgiIt->second);
	}

	m_poller.removeFd(clientFd);
	delete it->second;
	m_clients.erase(it);
}

void Server::checkTimeouts() {
	time_t now = time(NULL);

	// 1. Check CGI Timeouts (5 seconds max)
	std::vector<CgiHandler*> timedOutCgis;
	for (std::map<int, CgiHandler*>::iterator it = m_clientCgis.begin();
	     it != m_clientCgis.end(); ++it) {
		if (now - it->second->getStartTime() > 5) {
			timedOutCgis.push_back(it->second);
		}
	}

	for (size_t i = 0; i < timedOutCgis.size(); ++i) {
		CgiHandler* cgi = timedOutCgis[i];
		int clientFd = cgi->getClientFd();

		std::map<int, Client*>::iterator clientIt = m_clients.find(clientFd);
		if (clientIt != m_clients.end()) {
			HttpResponse timeoutRes = Router::buildErrorResponse(
			    504, *clientIt->second->getServerConfig(), "CGI Timeout");
			clientIt->second->setResponse(timeoutRes, false);
			m_poller.modifyFd(clientFd, EVENT_READ | EVENT_WRITE);
		}
		closeCgi(cgi);
	}

	// 2. Check Client Inactivity Timeouts (60 seconds)
	std::vector<int> timedOutClients;
	for (std::map<int, Client*>::iterator it = m_clients.begin();
	     it != m_clients.end(); ++it) {
		if (it->second->getState() != CLIENT_CGI_PROCESSING &&
		    (now - it->second->getLastActivity() > 60)) {
			timedOutClients.push_back(it->first);
		}
	}

	for (size_t i = 0; i < timedOutClients.size(); ++i) {
		closeClient(timedOutClients[i]);
	}

	// 3. Reap any terminated child processes
	int status = 0;
	while (waitpid(-1, &status, WNOHANG) > 0)
		;
}
