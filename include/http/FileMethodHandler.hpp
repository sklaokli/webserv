#ifndef FILEMETHODHANDLER_HPP
#define FILEMETHODHANDLER_HPP

#include "http/Router.hpp"

class FileMethodHandler {
public:
    // Full dechunked body; caller extracts Content-Type case-insensitively.
    // RESPONSE is ready to serialize, CGI/METHOD actions require another handler.
    static RouteResult handle(const std::string& method,
        const std::string& target, const std::string& contentType,
        const std::string& body, const ServerConfig& server);
};
#endif
