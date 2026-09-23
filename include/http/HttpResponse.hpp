#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <map>
#include <string>
struct HttpResponse {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;
    HttpResponse(int code = 200);
    // A HEAD response carries the GET Content-Length but no body on the wire.
    std::string serialize(bool head = false) const;
    static std::string reason(int status);
};
#endif