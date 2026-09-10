#include "Client.hpp"
#include <unistd.h>

Client::Client(
    int fd, const std::string& ip, int port, const ServerConfig* server)
    : m_fd(fd)
    , m_ip(ip)
    , m_port(port)
    , m_lastActivity(time(NULL))
    , m_state(CLIENT_READING)
    , m_server(server)
    , m_shouldClose(false) {}

Client::~Client() {
	if (m_fd != -1) {
		close(m_fd);
		m_fd = -1;
	}
}

int Client::getFd() const {
	return m_fd;
}
const std::string& Client::getIp() const {
	return m_ip;
}
int Client::getPort() const {
	return m_port;
}
time_t Client::getLastActivity() const {
	return m_lastActivity;
}

void Client::updateActivity() {
	m_lastActivity = time(NULL);
}

ClientState Client::getState() const {
	return m_state;
}
void Client::setState(ClientState state) {
	m_state = state;
}

void Client::setServerConfig(const ServerConfig* server) {
	m_server = server;
}
const ServerConfig* Client::getServerConfig() const {
	return m_server;
}

HttpRequest& Client::getRequest() {
	return m_request;
}
const HttpRequest& Client::getRequest() const {
	return m_request;
}

void Client::setResponse(const HttpResponse& response, bool keepAlive) {
	m_writeBuffer = response.serialize();
	m_shouldClose = !keepAlive;
	m_state = CLIENT_WRITING;
}

const std::string& Client::getWriteBuffer() const {
	return m_writeBuffer;
}

bool Client::hasDataToSend() const {
	return !m_writeBuffer.empty();
}

void Client::consumeWriteBuffer(size_t bytes) {
	if (bytes >= m_writeBuffer.length()) {
		m_writeBuffer.clear();
	} else {
		m_writeBuffer.erase(0, bytes);
	}
}

bool Client::shouldClose() const {
	return m_shouldClose;
}

void Client::setShouldClose(bool close) {
	m_shouldClose = close;
}

void Client::resetForNextRequest() {
	m_request.reset();
	m_writeBuffer.clear();
	m_state = CLIENT_READING;
	updateActivity();
}
