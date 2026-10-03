#include "http/FileMethodHandler.hpp"
#include "http/MultipartParser.hpp"
#include "http/PathResolver.hpp"
#include <cerrno>
#include <fcntl.h>
#include <fstream>
#include <set>
#include <unistd.h>
#include <vector>

static RouteResult failure(RouteResult result, int status, const ServerConfig& server) {
    result.action = RouteResult::RESPONSE;
    result.response = Router::error(status, server);
    return result;
}
static std::string uploadUrl(const LocationConfig& loc, const std::string& filename) {
    std::string prefix = loc.getPath();
    if (prefix.empty() || prefix[prefix.size() - 1] != '/') prefix += "/";
    return PathResolver::encodePath(prefix + filename);
}
static int writableDirectory(int fd, const struct stat& info) {
    if (!S_ISDIR(info.st_mode)) return 403;
    if (!(info.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH))) return 403;
    if (access(PathResolver::descriptorPath(fd).c_str(), W_OK | X_OK) != 0)
        return PathResolver::filesystemError(errno);
    return 200;
}
struct CreatedFile {
    std::string path;
    bool created;
    explicit CreatedFile(const std::string& value) : path(value), created(false) {}
};
// Directory fd is owned by the caller and outlives this rollback guard.
class UploadTransaction {
public:
    std::vector<CreatedFile> files;
    bool committed;
    UploadTransaction() : committed(false) {}
    ~UploadTransaction() {
        if (!committed)
            for (size_t i = 0; i < files.size(); ++i)
                if (files[i].created) unlink(files[i].path.c_str());
    }
private:
    UploadTransaction(const UploadTransaction&);
    UploadTransaction& operator=(const UploadTransaction&);
};
static int writeFile(CreatedFile& destination, const std::string& body,
    const UploadPart& part) {
    // Exclusive creation prevents overwriting existing files and following links.
    int fd = open(destination.path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        if (errno == EEXIST) return 409;
        return PathResolver::filesystemError(errno);
    }
    destination.created = true;
    FileHandle file(fd);
    struct stat info;
    std::string pinned = PathResolver::descriptorPath(fd);
    if (stat(pinned.c_str(), &info) != 0) return PathResolver::filesystemError(errno);
    if (!S_ISREG(info.st_mode)) return 403;
    // Open the already pinned inode, never reopen the client-controlled pathname.
    std::ofstream output(pinned.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!output) return 500;
    size_t written = 0;
    while (written < part.length) {
        size_t count = part.length - written;
        if (count > 16384) count = 16384;
        output.write(body.data() + part.offset + written, static_cast<std::streamsize>(count));
        if (!output) return 500;
        written += count;
    }
    output.close();
    return output.fail() ? 500 : 200;
}
static RouteResult upload(RouteResult result, const std::string& contentType,
    const std::string& body, const ServerConfig& server) {
    const LocationConfig* loc = result.location;
    if (!loc || !loc->getUploadEnable() || loc->getUploadStore().empty())
        return failure(result, 403, server);
    std::string media;
    std::map<std::string, std::string> parameters;
    int status = MultipartParser::parseParameters(contentType, media, parameters);
    if (status != 200) return failure(result, status, server);
    std::vector<UploadPart> parts;
    if (media == "multipart/form-data") {
        if (!parameters.count("boundary")) return failure(result, 400, server);
        status = MultipartParser::parse(body, parameters["boundary"], parts);
    } else if (media == "application/octet-stream") {
        std::string suffix = result.path.substr(loc->getPath().size());
        if (!suffix.empty() && suffix[0] == '/') suffix.erase(0, 1);
        // Flat storage: raw URL must name a direct child of the upload location.
        if (!MultipartParser::validFilename(suffix)) return failure(result, 403, server);
        parts.push_back(UploadPart(suffix, 0, body.size()));
    } else return failure(result, 415, server);
    if (status != 200) return failure(result, status, server);
    FileHandle directory;
    struct stat info;
    status = PathResolver::openBeneath(loc->getUploadStore(), "", directory, info);
    if (status == 200) status = writableDirectory(directory.get(), info);
    if (status != 200) return failure(result, status, server);

    UploadTransaction transaction;
    const std::string base = PathResolver::descriptorPath(directory.get()) + "/";
    // Allocate every pathname and the success response before changing the disk.
    for (size_t i = 0; i < parts.size(); ++i)
        transaction.files.push_back(CreatedFile(base + parts[i].filename));
    result.action = RouteResult::RESPONSE;
    result.response = HttpResponse(201);
    result.response.headers["Location"] = uploadUrl(*loc, parts[0].filename);
    if (parts.size() > 1) {
        result.response.headers["Content-Type"] = "text/plain; charset=utf-8";
        for (size_t i = 0; i < parts.size(); ++i)
            result.response.body += uploadUrl(*loc, parts[i].filename) + "\n";
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        status = writeFile(transaction.files[i], body, parts[i]);
        if (status != 200) return failure(result, status, server);
    }
    transaction.committed = true;
    return result;
}
static RouteResult deleteFile(RouteResult result, const ServerConfig& server) {
    // DELETE is controlled by allowed_methods; upload_enable applies only to POST.
    std::string relative = result.relativePath;
    if (relative.empty() || relative[relative.size() - 1] == '/') {
        FileHandle directory;
        struct stat info;
        int status = PathResolver::openBeneath(result.base, relative, directory, info);
        return failure(result, status == 200 ? 403 : status, server);
    }
    size_t slash = relative.find_last_of('/');
    std::string parent = slash == std::string::npos ? "" : relative.substr(0, slash);
    std::string name = slash == std::string::npos ? relative : relative.substr(slash + 1);
    if (!MultipartParser::validFilename(name)) return failure(result, 403, server);
    FileHandle directory;
    struct stat parentInfo;
    int status = PathResolver::openBeneath(result.base, parent, directory, parentInfo);
    if (status != 200) return failure(result, status, server);
    FileHandle target;
    struct stat info;
    status = PathResolver::openChild(directory.get(), name, target, info);
    if (status != 200) return failure(result, status, server);
    if (!S_ISREG(info.st_mode)) return failure(result, 403, server);
    status = writableDirectory(directory.get(), parentInfo);
    if (status != 200) return failure(result, status, server);
    // Parent stays pinned. unlink never follows a replacement leaf symlink.
    std::string path = PathResolver::descriptorPath(directory.get()) + "/" + name;
    result.action = RouteResult::RESPONSE;
    result.response = HttpResponse(204);
    if (unlink(path.c_str()) != 0) {
        if (errno == EISDIR) return failure(result, 403, server);
        return failure(result, PathResolver::filesystemError(errno), server);
    }
    return result;
}

RouteResult FileMethodHandler::handle(const std::string& method,
    const std::string& target, const std::string& contentType,
    const std::string& body, const ServerConfig& server) {
    RouteResult result = Router::route(method, target, server);
    if (result.action != RouteResult::METHOD_HANDLER) return result;
    if (method != "POST" && method != "DELETE") return result;
    const LocationConfig* loc = result.location;
    if (method == "POST") {
        size_t limit = loc ? loc->getClientMaxBodySize() : server.getClientMaxBodySize();
        if (limit && body.size() > limit) return failure(result, 413, server);
        size_t dot = result.path.find_last_of('.');
        size_t slash = result.path.find_last_of('/');
        if (loc && dot != std::string::npos && dot > slash &&
            loc->hasCgi(result.path.substr(dot))) {
            result.action = RouteResult::CGI_HANDLER;
            return result;
        }
        return upload(result, contentType, body, server);
    }
    return deleteFile(result, server);
}
