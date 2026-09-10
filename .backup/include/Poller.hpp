#ifndef POLLER_HPP
#define POLLER_HPP

#include <map>
#include <sys/epoll.h>
#include <vector>

enum EventType {
	EVENT_NONE = 0,
	EVENT_READ = 1 << 0,
	EVENT_WRITE = 1 << 1,
	EVENT_ERROR = 1 << 2
};

struct PollEvent {
	int fd;
	int events;  // bitmask of EventType
};

class Poller {
public:
	Poller();
	~Poller();

	bool init();
	bool addFd(int fd, int events);
	bool modifyFd(int fd, int events);
	bool removeFd(int fd);
	int pollEvents(std::vector<PollEvent>& triggeredEvents, int timeoutMs);

private:
	int m_epollFd;
	std::map<int, int> m_registeredEvents;  // fd -> mask

	uint32_t toEpollEvents(int events);
	int toEventMask(uint32_t epollEvents);
};

#endif
