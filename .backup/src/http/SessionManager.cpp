#include "SessionManager.hpp"
#include <cstdlib>
#include <iomanip>
#include <sstream>

std::map<std::string, Session> SessionManager::s_sessions;

Session::Session()
    : id(""), username(""), visitCount(0), createdAt(0), lastAccessed(0) {}

Session::Session(const std::string& sessId, const std::string& user)
    : id(sessId)
    , username(user)
    , visitCount(1)
    , createdAt(time(NULL))
    , lastAccessed(time(NULL)) {}

std::string SessionManager::generateSessionId() {
	static unsigned long counter = 0;
	std::ostringstream oss;

	time_t now = time(NULL);
	int r1 = std::rand();
	int r2 = std::rand();
	counter++;

	oss << "sess_" << std::hex << now << "_" << std::setfill('0')
	    << std::setw(8) << r1 << std::setw(8) << r2 << std::setw(4) << counter;

	return oss.str();
}

Session* SessionManager::createSession(const std::string& username) {
	std::string id = generateSessionId();
	Session session(id, username.empty() ? "Guest" : username);
	s_sessions[id] = session;
	return &s_sessions[id];
}

Session* SessionManager::getSession(const std::string& sessionId) {
	if (sessionId.empty()) return NULL;

	std::map<std::string, Session>::iterator it = s_sessions.find(sessionId);
	if (it != s_sessions.end()) {
		it->second.lastAccessed = time(NULL);
		it->second.visitCount++;
		return &it->second;
	}
	return NULL;
}

bool SessionManager::destroySession(const std::string& sessionId) {
	if (sessionId.empty()) return false;
	std::map<std::string, Session>::iterator it = s_sessions.find(sessionId);
	if (it != s_sessions.end()) {
		s_sessions.erase(it);
		return true;
	}
	return false;
}

void SessionManager::cleanupExpired(time_t maxAgeSeconds) {
	time_t now = time(NULL);
	std::map<std::string, Session>::iterator it = s_sessions.begin();
	while (it != s_sessions.end()) {
		if (now - it->second.lastAccessed > maxAgeSeconds) {
			std::map<std::string, Session>::iterator toErase = it++;
			s_sessions.erase(toErase);
		} else {
			++it;
		}
	}
}

size_t SessionManager::getActiveSessionCount() {
	return s_sessions.size();
}
