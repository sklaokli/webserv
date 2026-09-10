#include "Poller.hpp"
#include "Logger.hpp"
#include <cerrno>
#include <cstring>
#include <unistd.h>

Poller::Poller() : m_epollFd(-1) {}

Poller::~Poller() {
	if (m_epollFd != -1) {
		close(m_epollFd);
		m_epollFd = -1;
	}
}

bool Poller::init() {
	m_epollFd = epoll_create(1024);
	if (m_epollFd == -1) {
		Logger::error("epoll_create failed: " + std::string(strerror(errno)));
		return false;
	}
	return true;
}

uint32_t Poller::toEpollEvents(int events) {
	uint32_t epollEvents = 0;
	if (events & EVENT_READ) epollEvents |= EPOLLIN;
	if (events & EVENT_WRITE) epollEvents |= EPOLLOUT;
	if (events & EVENT_ERROR) epollEvents |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP);
	return epollEvents;
}

int Poller::toEventMask(uint32_t epollEvents) {
	int mask = EVENT_NONE;
	if (epollEvents & (EPOLLIN | EPOLLPRI)) mask |= EVENT_READ;
	if (epollEvents & EPOLLOUT) mask |= EVENT_WRITE;
	if (epollEvents & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) mask |= EVENT_ERROR;
	return mask;
}

bool Poller::addFd(int fd, int events) {
	if (m_epollFd == -1 || fd < 0) return false;

	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.data.fd = fd;
	ev.events = toEpollEvents(events);

	if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		return false;
	}

	m_registeredEvents[fd] = events;
	return true;
}

bool Poller::modifyFd(int fd, int events) {
	if (m_epollFd == -1 || fd < 0) return false;

	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.data.fd = fd;
	ev.events = toEpollEvents(events);

	if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		return false;
	}

	m_registeredEvents[fd] = events;
	return true;
}

bool Poller::removeFd(int fd) {
	if (m_epollFd == -1 || fd < 0) return false;

	m_registeredEvents.erase(fd);
	if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		return false;
	}
	return true;
}

int Poller::pollEvents(std::vector<PollEvent>& triggeredEvents, int timeoutMs) {
	triggeredEvents.clear();
	if (m_epollFd == -1) return -1;

	const int MAX_EVENTS = 128;
	struct epoll_event events[MAX_EVENTS];

	int numFds = epoll_wait(m_epollFd, events, MAX_EVENTS, timeoutMs);
	if (numFds < 0) {
		return numFds;
	}

	for (int i = 0; i < numFds; ++i) {
		PollEvent pe;
		pe.fd = events[i].data.fd;
		pe.events = toEventMask(events[i].events);
		triggeredEvents.push_back(pe);
	}

	return numFds;
}
