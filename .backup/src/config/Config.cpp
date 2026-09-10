#include "Config.hpp"
#include <algorithm>

LocationConfig::LocationConfig()
    : path("/")
    , root("")
    , index("index.html")
    , autoindex(false)
    , redirect_code(0)
    , redirect_url("")
    , upload_enable(false)
    , upload_store("")
    , client_max_body_size(0) {
	allowed_methods.insert("GET");
	allowed_methods.insert("POST");
	allowed_methods.insert("DELETE");
}

bool LocationConfig::isMethodAllowed(const std::string& method) const {
	if (allowed_methods.empty()) return true;
	return allowed_methods.find(method) != allowed_methods.end();
}

bool LocationConfig::hasCgiExtension(const std::string& ext) const {
	return cgi_handlers.find(ext) != cgi_handlers.end();
}

std::string LocationConfig::getCgiInterpreter(const std::string& ext) const {
	std::map<std::string, std::string>::const_iterator it =
	    cgi_handlers.find(ext);
	if (it != cgi_handlers.end()) return it->second;
	return "";
}

ServerConfig::ServerConfig()
    : host("0.0.0.0")
    , port(8080)
    , client_max_body_size(10 * 1024 * 1024)
    ,  // 10MB default
    root("./www")
    , index("index.html") {}

bool ServerConfig::hasServerName(const std::string& name) const {
	for (size_t i = 0; i < server_names.size(); ++i) {
		if (server_names[i] == name) return true;
	}
	return false;
}

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const {
	const LocationConfig* bestMatch = NULL;
	size_t longestMatchLen = 0;

	for (size_t i = 0; i < locations.size(); ++i) {
		const std::string& locPath = locations[i].path;

		bool isMatch = false;
		if (locPath == "/" && uri[0] == '/') {
			isMatch = true;
		} else if (uri == locPath) {
			isMatch = true;
		} else if (uri.find(locPath) == 0) {
			// Ensure prefix match is at boundary: either locPath ends with '/'
			// or uri has '/' right after locPath
			if (locPath[locPath.length() - 1] == '/' ||
			    uri[locPath.length()] == '/') {
				isMatch = true;
			}
		}

		if (isMatch && locPath.length() > longestMatchLen) {
			longestMatchLen = locPath.length();
			bestMatch = &locations[i];
		}
	}

	return bestMatch;
}

std::string ServerConfig::getErrorPage(int statusCode) const {
	std::map<int, std::string>::const_iterator it =
	    error_pages.find(statusCode);
	if (it != error_pages.end()) return it->second;
	return "";
}

Config::Config() {}
Config::~Config() {}

void Config::addServer(const ServerConfig& server) {
	m_servers.push_back(server);
}

const std::vector<ServerConfig>& Config::getServers() const {
	return m_servers;
}

const ServerConfig* Config::findServer(
    const std::string& host, int port, const std::string& serverName) const {
	const ServerConfig* defaultForPort = NULL;

	for (size_t i = 0; i < m_servers.size(); ++i) {
		if (m_servers[i].port == port &&
		    (m_servers[i].host == host || m_servers[i].host == "0.0.0.0" ||
		        host == "0.0.0.0")) {
			if (!defaultForPort) defaultForPort = &m_servers[i];
			if (!serverName.empty() && m_servers[i].hasServerName(serverName))
				return &m_servers[i];
		}
	}

	return defaultForPort;
}
