#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "config/ServerConfig.hpp"
#include "http/HttpResponse.hpp"

struct RouteResult {
    enum Action { RESPONSE, METHOD_HANDLER, CGI_HANDLER };
    Action action;
    HttpResponse response;
    // Non-owning: ServerConfig must outlive the result.
    const LocationConfig* location;
    std::string path;
    std::string query;
    std::string base;
    std::string relativePath;
    RouteResult();
};

class Router {
public:
    // target is the original origin-form request target, not pre-decoded.
    static RouteResult route(const std::string& method,
        const std::string& target, const ServerConfig& server);
    static HttpResponse error(int status, const ServerConfig& server);
};
#endif