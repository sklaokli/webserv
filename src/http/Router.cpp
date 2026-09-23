#include "http/Router.hpp"
#include "http/AutoIndex.hpp"
#include "http/MimeTypes.hpp"
#include "http/PathResolver.hpp"

RouteResult::RouteResult() : action(RESPONSE), location(NULL) {}
namespace {
std::string extension(const std::string& path) {
    size_t slash = path.find_last_of('/'), dot = path.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return "";
    return path.substr(dot);
}
std::string allowHeader(const LocationConfig* loc) {
    if (!loc || loc->getAllowedMethods().empty()) return "GET";
    std::string out;
    const std::vector<std::string>& methods = loc->getAllowedMethods();
    for (size_t i = 0; i < methods.size(); ++i) {
        if (i) out += ", ";
        out += methods[i];
    }
    return out;
}
bool headerSafe(const std::string& value) {
    for (size_t i = 0; i < value.size(); ++i)
        if (static_cast<unsigned char>(value[i]) < 32 || value[i] == 127) return false;
    return true;
}
}
HttpResponse Router::error(int status, const ServerConfig& server) {
    HttpResponse response(status);
    response.headers["Content-Type"] = "text/html; charset=utf-8";
    response.headers["X-Content-Type-Options"] = "nosniff";
    std::string page = server.getErrorPage(status);
    if (!page.empty()) {
        std::string normalized, query;
        FileHandle file;
        struct stat info;
        if (PathResolver::normalizeTarget(page, normalized, query) == 200 &&
            query.empty() &&
            PathResolver::openBeneath(server.getRoot(), normalized, file, info) == 200 &&
            S_ISREG(info.st_mode) &&
            PathResolver::readRegular(file.get(), response.body) == 200) {
            response.headers["Content-Type"] = MimeTypes::lookup(normalized);
            return response;
        }
    }
    const std::string title = HttpResponse::reason(status);
    response.body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        "<title>" + title + "</title></head><body><h1>" + title +
        "</h1></body></html>\n";
    return response;
}

RouteResult Router::route(const std::string& method,
    const std::string& target, const ServerConfig& server) {
    RouteResult result;
    int status = PathResolver::normalizeTarget(target, result.path, result.query);
    if (status != 200) { result.response = error(status, server); return result; }
    result.location = server.findLocation(result.path);
    const LocationConfig* loc = result.location;
    if ((loc && !loc->isMethodAllowed(method)) || (!loc && method != "GET")) {
        result.response = error(405, server);
        result.response.headers["Allow"] = allowHeader(loc);
        return result;
    }
    if (loc && loc->hasRedirect()) {
        if (!headerSafe(loc->getRedirectUrl())) {
            result.response = error(500, server);
            return result;
        }
        result.response.status = loc->getRedirectCode();
        result.response.headers["Location"] = loc->getRedirectUrl();
        return result;
    }
    result.base = loc ? loc->getRoot() : server.getRoot();
    result.relativePath = result.path;
    if (loc && !loc->getAlias().empty()) {
        result.base = loc->getAlias();
        result.relativePath = result.path.substr(loc->getPath().size());
    }
    if (method != "GET" && method != "HEAD") {
        result.action = RouteResult::METHOD_HANDLER;
        return result;
    }
    // Never expose configured CGI source code as static content.
    if (loc && loc->hasCgi(extension(result.path))) {
        result.action = RouteResult::CGI_HANDLER;
        return result;
    }
    FileHandle file;
    struct stat info;
    status = PathResolver::openBeneath(result.base, result.relativePath, file, info);
    if (status != 200) { result.response = error(status, server); return result; }
    std::string contentPath = result.path;
    if (S_ISDIR(info.st_mode)) {
        if (result.path[result.path.size() - 1] != '/') {
            result.response.status = 301;
            result.response.headers["Location"] =
                PathResolver::encodePath(result.path) + "/" + result.query;
            return result;
        }
        std::string index = loc ? loc->getIndex() : server.getIndex();
        if (index.empty()) index = "index.html";
        FileHandle indexFile;
        struct stat indexInfo;
        status = PathResolver::openChild(file.get(), index, indexFile, indexInfo);
        if (status == 200 && S_ISREG(indexInfo.st_mode)) {
            contentPath += index;
            if (loc && loc->hasCgi(extension(contentPath))) {
                result.relativePath += "/" + index;
                result.path = contentPath;
                result.action = RouteResult::CGI_HANDLER;
                return result;
            }
            status = PathResolver::readRegular(indexFile.get(), result.response.body);
        } else if (status == 404) {
            if (!loc || !loc->getAutoindex()) status = 403;
            else {
                status = AutoIndex::generate(file.get(), result.path, result.response.body);
                if (status == 200) {
                    result.response.headers["Content-Type"] = "text/html; charset=utf-8";
                    result.response.headers["X-Content-Type-Options"] = "nosniff";
                    return result;
                }
            }
        } else if (status == 200) status = 403;
    } else if (result.path[result.path.size() - 1] == '/') status = 404;
    else status = PathResolver::readRegular(file.get(), result.response.body);
    if (status != 200) { result.response = error(status, server); return result; }
    result.response.headers["Content-Type"] = MimeTypes::lookup(contentPath);
    result.response.headers["X-Content-Type-Options"] = "nosniff";
    return result;
}