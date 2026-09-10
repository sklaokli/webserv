#include "CgiHandler.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

CgiHandler::CgiHandler()
    : m_pid(-1)
    , m_inPipeFd(-1)
    , m_outPipeFd(-1)
    , m_clientFd(-1)
    , m_startTime(0)
    , m_isOutputComplete(false)
    , m_hasError(false) {}

CgiHandler::~CgiHandler() {
	cleanup();
}

int CgiHandler::getInPipeFd() const {
	return m_inPipeFd;
}
int CgiHandler::getOutPipeFd() const {
	return m_outPipeFd;
}
int CgiHandler::getClientFd() const {
	return m_clientFd;
}
pid_t CgiHandler::getPid() const {
	return m_pid;
}
time_t CgiHandler::getStartTime() const {
	return m_startTime;
}

bool CgiHandler::hasInputToWrite() const {
	return (m_inPipeFd != -1 && !m_inputBuffer.empty());
}

bool CgiHandler::isOutputComplete() const {
	return m_isOutputComplete;
}

std::vector<std::string> CgiHandler::buildEnv(const HttpRequest& request,
    const LocationConfig& loc, const ServerConfig& server,
    const std::string& scriptPath, const std::string& clientIp) {
	(void)loc;
	std::vector<std::string> env;

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_SOFTWARE=Webserv/1.0");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back(
	    "SERVER_NAME=" +
	    (server.server_names.empty() ? server.host : server.server_names[0]));
	env.push_back("SERVER_PORT=" + Utils::toString(server.port));
	env.push_back("REQUEST_METHOD=" + request.getMethod());
	env.push_back("REQUEST_URI=" + request.getUri());
	env.push_back("SCRIPT_NAME=" + request.getPath());
	env.push_back("SCRIPT_FILENAME=" + scriptPath);
	env.push_back("PATH_INFO=" + request.getPath());
	env.push_back("PATH_TRANSLATED=" + scriptPath);
	env.push_back("QUERY_STRING=" + request.getQueryString());
	env.push_back("REMOTE_ADDR=" + clientIp);
	env.push_back("REDIRECT_STATUS=200");  // Needed for php-cgi

	if (request.hasHeader("content-length")) {
		env.push_back("CONTENT_LENGTH=" + request.getHeader("content-length"));
	} else if (!request.getBody().empty()) {
		env.push_back(
		    "CONTENT_LENGTH=" + Utils::toString(request.getBody().length()));
	}

	if (request.hasHeader("content-type")) {
		env.push_back("CONTENT_TYPE=" + request.getHeader("content-type"));
	}

	// Convert request headers to HTTP_* env variables
	const std::map<std::string, std::string>& headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it =
	         headers.begin();
	     it != headers.end(); ++it) {
		std::string varName = "HTTP_";
		for (size_t i = 0; i < it->first.length(); ++i) {
			char c = it->first[i];
			if (c == '-')
				varName += '_';
			else
				varName += std::toupper(static_cast<unsigned char>(c));
		}
		env.push_back(varName + "=" + it->second);
	}

	return env;
}

char** CgiHandler::createEnvp(const std::vector<std::string>& envVec) {
	char** envp = new char*[envVec.size() + 1];
	for (size_t i = 0; i < envVec.size(); ++i) {
		char* str = new char[envVec[i].length() + 1];
		std::strcpy(str, envVec[i].c_str());
		envp[i] = str;
	}
	envp[envVec.size()] = NULL;
	return envp;
}

void CgiHandler::freeEnvp(char** envp) {
	if (!envp) return;
	for (size_t i = 0; envp[i] != NULL; ++i) {
		delete[] envp[i];
	}
	delete[] envp;
}

bool CgiHandler::start(const HttpRequest& request, const LocationConfig& loc,
    const ServerConfig& server, const std::string& scriptPath,
    const std::string& interpreter, int clientFd, const std::string& clientIp) {
	m_clientFd = clientFd;
	m_inputBuffer = request.getBody();
	m_startTime = time(NULL);

	int inPipe[2];
	int outPipe[2];

	if (pipe(inPipe) == -1) {
		Logger::error("Failed to create CGI in-pipe");
		return false;
	}

	if (pipe(outPipe) == -1) {
		Logger::error("Failed to create CGI out-pipe");
		close(inPipe[0]);
		close(inPipe[1]);
		return false;
	}

	// Set non-blocking and close-on-exec on parent ends
	fcntl(inPipe[1], F_SETFL, O_NONBLOCK);
	fcntl(inPipe[1], F_SETFD, FD_CLOEXEC);
	fcntl(outPipe[0], F_SETFL, O_NONBLOCK);
	fcntl(outPipe[0], F_SETFD, FD_CLOEXEC);

	m_pid = fork();
	if (m_pid < 0) {
		Logger::error("fork failed for CGI");
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);
		return false;
	}

	if (m_pid == 0) {
		// In Child Process
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);

		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);

		// Change directory to script directory
		std::string scriptDir = ".";
		size_t lastSlash = scriptPath.find_last_of('/');
		if (lastSlash != std::string::npos) {
			scriptDir = scriptPath.substr(0, lastSlash);
		}
		if (chdir(scriptDir.c_str()) != 0) {
			std::exit(1);
		}

		std::vector<std::string> envVec =
		    buildEnv(request, loc, server, scriptPath, clientIp);
		char** envp = createEnvp(envVec);

		std::string scriptFile = scriptPath;
		if (lastSlash != std::string::npos) {
			scriptFile = scriptPath.substr(lastSlash + 1);
		}

		char* argv[3];
		argv[0] = const_cast<char*>(interpreter.c_str());
		argv[1] = const_cast<char*>(scriptFile.c_str());
		argv[2] = NULL;

		execve(interpreter.c_str(), argv, envp);
		std::exit(1);
	}

	// In Parent Process
	close(inPipe[0]);
	close(outPipe[1]);

	m_inPipeFd = inPipe[1];
	m_outPipeFd = outPipe[0];

	// If there is no input to write, close inPipeFd immediately so child sees
	// EOF
	if (m_inputBuffer.empty()) {
		close(m_inPipeFd);
		m_inPipeFd = -1;
	}

	return true;
}

void CgiHandler::writeInputChunk() {
	if (m_inPipeFd == -1 || m_inputBuffer.empty()) return;

	ssize_t bytesWritten =
	    write(m_inPipeFd, m_inputBuffer.c_str(), m_inputBuffer.length());
	if (bytesWritten > 0) {
		m_inputBuffer.erase(0, bytesWritten);
	}

	if (m_inputBuffer.empty()) {
		close(m_inPipeFd);
		m_inPipeFd = -1;
	}
}

void CgiHandler::readOutputChunk() {
	if (m_outPipeFd == -1) return;

	char buf[4096];
	ssize_t bytesRead = read(m_outPipeFd, buf, sizeof(buf));
	if (bytesRead > 0) {
		m_outputBuffer.append(buf, bytesRead);
	} else if (bytesRead == 0) {
		// EOF reached
		close(m_outPipeFd);
		m_outPipeFd = -1;
		m_isOutputComplete = true;

		if (m_pid > 0) {
			int status = 0;
			waitpid(m_pid, &status, WNOHANG);
			m_pid = -1;
		}
	} else {
		// Error on pipe
		close(m_outPipeFd);
		m_outPipeFd = -1;
		m_hasError = true;
		m_isOutputComplete = true;
	}
}

HttpResponse CgiHandler::buildResponse() {
	HttpResponse res;
	if (m_hasError) {
		res.setStatus(502, "Bad Gateway");
		res.setBody(
		    HttpResponse::generateDefaultErrorPage(502, "CGI execution error"));
		return res;
	}

	// Parse CGI headers and body
	size_t headerEnd = m_outputBuffer.find("\r\n\r\n");
	size_t headerDelimLen = 4;
	if (headerEnd == std::string::npos) {
		headerEnd = m_outputBuffer.find("\n\n");
		headerDelimLen = 2;
	}

	if (headerEnd == std::string::npos) {
		// No headers returned, treat everything as body
		res.setStatus(200);
		res.setHeader("Content-Type", "text/html");
		res.setBody(m_outputBuffer);
		return res;
	}

	std::string headerSection = m_outputBuffer.substr(0, headerEnd);
	std::string bodySection = m_outputBuffer.substr(headerEnd + headerDelimLen);

	int statusCode = 200;
	std::string statusMsg = "OK";

	std::vector<std::string> lines = Utils::split(headerSection, '\n');
	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = Utils::trim(lines[i]);
		if (line.empty()) continue;

		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string key = Utils::trim(line.substr(0, colon));
			std::string val = Utils::trim(line.substr(colon + 1));

			if (Utils::toLower(key) == "status") {
				std::vector<std::string> statusParts =
				    Utils::splitWhitespace(val);
				if (!statusParts.empty()) {
					statusCode = Utils::toInt(statusParts[0]);
					if (statusParts.size() > 1) {
						statusMsg = val.substr(statusParts[0].length() + 1);
					}
				}
			} else {
				res.setHeader(key, val);
			}
		}
	}

	res.setStatus(statusCode, statusMsg);
	if (!res.hasHeader("Content-Type")) {
		res.setHeader("Content-Type", "text/html");
	}
	res.setBody(bodySection);
	return res;
}

void CgiHandler::killChild() {
	if (m_pid > 0) {
		kill(m_pid, SIGKILL);
		int status = 0;
		waitpid(m_pid, &status, 0);  // Cleanly reap killed child process
		m_pid = -1;
	}
}

void CgiHandler::cleanup() {
	if (m_inPipeFd != -1) {
		close(m_inPipeFd);
		m_inPipeFd = -1;
	}
	if (m_outPipeFd != -1) {
		close(m_outPipeFd);
		m_outPipeFd = -1;
	}
	killChild();
}
