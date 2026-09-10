#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

class CgiHandler {
public:
	CgiHandler();
	~CgiHandler();

	bool start(const HttpRequest& request, const LocationConfig& loc,
	    const ServerConfig& server, const std::string& scriptPath,
	    const std::string& interpreter, int clientFd,
	    const std::string& clientIp);

	int getInPipeFd() const;
	int getOutPipeFd() const;
	int getClientFd() const;
	pid_t getPid() const;
	time_t getStartTime() const;

	bool hasInputToWrite() const;
	void writeInputChunk();

	void readOutputChunk();
	bool isOutputComplete() const;

	HttpResponse buildResponse();
	void killChild();
	void cleanup();

private:
	pid_t m_pid;
	int m_inPipeFd;   // parent writes request body to this
	int m_outPipeFd;  // parent reads CGI output from this
	int m_clientFd;
	time_t m_startTime;

	std::string m_inputBuffer;
	std::string m_outputBuffer;
	bool m_isOutputComplete;
	bool m_hasError;

	std::vector<std::string> buildEnv(const HttpRequest& request,
	    const LocationConfig& loc, const ServerConfig& server,
	    const std::string& scriptPath, const std::string& clientIp);
	char** createEnvp(const std::vector<std::string>& envVec);
	void freeEnvp(char** envp);
};

#endif
