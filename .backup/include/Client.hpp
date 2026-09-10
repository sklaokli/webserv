#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <string>

enum ClientState {
	CLIENT_READING,
	CLIENT_PROCESSING,
	CLIENT_WRITING,
	CLIENT_CGI_PROCESSING,
	CLIENT_CLOSING
};

class Client {
public:
	Client(int fd, const std::string& ip, int port, const ServerConfig* server);
	~Client();

	int getFd() const;
	const std::string& getIp() const;
	int getPort() const;
	time_t getLastActivity() const;
	void updateActivity();

	ClientState getState() const;
	void setState(ClientState state);

	void setServerConfig(const ServerConfig* server);
	const ServerConfig* getServerConfig() const;

	HttpRequest& getRequest();
	const HttpRequest& getRequest() const;

	void setResponse(const HttpResponse& response, bool keepAlive);
	const std::string& getWriteBuffer() const;
	bool hasDataToSend() const;
	void consumeWriteBuffer(size_t bytes);

	bool shouldClose() const;
	void setShouldClose(bool close);

	void resetForNextRequest();

private:
	int m_fd;
	std::string m_ip;
	int m_port;
	time_t m_lastActivity;

	ClientState m_state;
	const ServerConfig* m_server;

	HttpRequest m_request;
	std::string m_writeBuffer;
	bool m_shouldClose;
};

#endif
