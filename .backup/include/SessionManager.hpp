#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <ctime>
#include <map>
#include <string>

struct Session {
	std::string id;
	std::string username;
	int visitCount;
	time_t createdAt;
	time_t lastAccessed;

	Session();
	Session(const std::string& id, const std::string& username);
};

class SessionManager {
public:
	static Session* createSession(const std::string& username = "Guest");
	static Session* getSession(const std::string& sessionId);
	static bool destroySession(const std::string& sessionId);
	static void cleanupExpired(time_t maxAgeSeconds = 3600);
	static size_t getActiveSessionCount();

private:
	static std::map<std::string, Session> s_sessions;
	static std::string generateSessionId();
};

#endif
