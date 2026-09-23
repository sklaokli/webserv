#include "http/MimeTypes.hpp"
#include <cctype>
#include <map>
namespace {
std::map<std::string, std::string> makeTypes() {
    std::map<std::string, std::string> m;
    m["html"] = m["htm"] = "text/html";
    m["css"] = "text/css";
    m["js"] = m["mjs"] = "text/javascript";
    m["txt"] = "text/plain";
    m["csv"] = "text/csv";
    m["json"] = "application/json";
    m["xml"] = "application/xml";
    m["pdf"] = "application/pdf";
    m["png"] = "image/png";
    m["jpg"] = m["jpeg"] = "image/jpeg";
    m["gif"] = "image/gif";
    m["svg"] = "image/svg+xml";
    m["ico"] = "image/x-icon";
    m["webp"] = "image/webp";
    m["avif"] = "image/avif";
    m["woff"] = "font/woff";
    m["woff2"] = "font/woff2";
    m["mp3"] = "audio/mpeg";
    m["mp4"] = "video/mp4";
    m["wasm"] = "application/wasm";
    m["zip"] = "application/zip";
    return m;
}
}
std::string MimeTypes::lookup(const std::string& path) {
    static const std::map<std::string, std::string> types = makeTypes();
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of('/');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return "application/octet-stream";
    std::string ext = path.substr(dot + 1);
    for (size_t i = 0; i < ext.size(); ++i)
        ext[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));
    std::map<std::string, std::string>::const_iterator it = types.find(ext);
    return it == types.end() ? "application/octet-stream" : it->second;
}