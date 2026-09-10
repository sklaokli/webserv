#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct LocationConfig {
	std::string path;
	std::string root;
	std::string index;
	std::set<std::string> allowed_methods;
	bool autoindex;
	int redirect_code;
	std::string redirect_url;
	bool upload_enable;
	std::string upload_store;
	size_t client_max_body_size;  // in bytes (0 = inherit from server)
	std::map<std::string, std::string>
	    cgi_handlers;  // e.g. ".py" -> "/usr/bin/python3"

	LocationConfig();
	bool isMethodAllowed(const std::string& method) const;
	bool hasCgiExtension(const std::string& ext) const;
	std::string getCgiInterpreter(const std::string& ext) const;
};

struct ServerConfig {
	std::string host;
	int port;
	std::vector<std::string> server_names;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;  // in bytes (0 = unlimited or default 10MB)
	std::string root;
	std::string index;
	std::vector<LocationConfig> locations;

	ServerConfig();
	const LocationConfig* findLocation(const std::string& uri) const;
	std::string getErrorPage(int statusCode) const;
	bool hasServerName(const std::string& name) const;
};

class Config {
public:
	Config();
	~Config();

	void addServer(const ServerConfig& server);
	const std::vector<ServerConfig>& getServers() const;
	const ServerConfig* findServer(const std::string& host, int port,
	    const std::string& serverName = "") const;

private:
	std::vector<ServerConfig> m_servers;
};

#endif
