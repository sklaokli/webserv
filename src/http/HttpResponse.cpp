#include "http/HttpResponse.hpp"
#include <sstream>
#include <stdexcept>

HttpResponse::HttpResponse(int code) : status(code) {}
std::string HttpResponse::reason(int code) {
    switch (code) {
    case 200: return "OK";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";
    case 400: return "Bad Request";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 414: return "URI Too Long";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    default: return "Response";
    }
}
std::string HttpResponse::serialize(bool head) const {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << " " << reason(status) << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        if (it->first.find_first_of("\r\n:") != std::string::npos ||
            it->second.find_first_of("\r\n") != std::string::npos)
            throw std::runtime_error("Invalid response header");
        if (it->first != "Content-Length")
            out << it->first << ": " << it->second << "\r\n";
    }
    if (status != 304) out << "Content-Length: " << body.size() << "\r\n";
    out << "\r\n";
    if (!head && status != 304) out << body;
    return out.str();
}