#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "Config.hpp"
#include <string>
#include <vector>

class ConfigParser {
public:
	ConfigParser();
	~ConfigParser();

	Config parse(const std::string& filePath);

private:
	std::vector<std::string> tokenize(const std::string& content);
	void parseServer(
	    const std::vector<std::string>& tokens, size_t& index, Config& config);
	void parseLocation(const std::vector<std::string>& tokens, size_t& index,
	    ServerConfig& server);
	void parseDirective(const std::vector<std::string>& tokens, size_t& index,
	    ServerConfig& server);
	void parseLocationDirective(const std::vector<std::string>& tokens,
	    size_t& index, LocationConfig& location);
};

#endif
