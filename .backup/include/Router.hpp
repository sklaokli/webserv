#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>

class Router {
public:
	Router();
	~Router();

	static HttpResponse handleRequest(
	    const HttpRequest& request, const ServerConfig& server);
	static HttpResponse buildErrorResponse(int statusCode,
	    const ServerConfig& server, const std::string& customMsg = "");
	static std::string resolvePath(
	    const std::string& uri, const LocationConfig& loc);

	static bool isCgiRequest(const HttpRequest& request,
	    const LocationConfig* loc, std::string& scriptPath,
	    std::string& interpreter);

private:
	static HttpResponse handleGet(const HttpRequest& request,
	    const LocationConfig& loc, const ServerConfig& server,
	    const std::string& fullPath);
	static HttpResponse handlePost(const HttpRequest& request,
	    const LocationConfig& loc, const ServerConfig& server,
	    const std::string& fullPath);
	static HttpResponse handleDelete(const HttpRequest& request,
	    const LocationConfig& loc, const ServerConfig& server,
	    const std::string& fullPath);
	static bool handleSessionApi(
	    const HttpRequest& request, HttpResponse& response);
};

#endif
