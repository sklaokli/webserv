#include "http/PathResolver.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <vector>

FileHandle::FileHandle(int fd) : _fd(fd) {}
FileHandle::~FileHandle() { if (_fd >= 0) close(_fd); }
int FileHandle::get() const { return _fd; }
void FileHandle::reset(int fd) {
    if (_fd >= 0) close(_fd);
    _fd = fd;
}

namespace {
int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
bool unsafeByte(unsigned char c) { return c < 32 || c == 127 || c == '\\'; }
bool safeComponent(const std::string& name) {
    return !name.empty() && name != "." && name != ".." &&
        name.find('/') == std::string::npos &&
        name.find('\\') == std::string::npos &&
        name.find('\0') == std::string::npos;
}
int inspect(int fd, struct stat& info) {
    const std::string path = PathResolver::descriptorPath(fd);
    if (stat(path.c_str(), &info) != 0)
        return PathResolver::filesystemError(errno);
    if (!S_ISDIR(info.st_mode) && !S_ISREG(info.st_mode)) return 403;
    if (!(info.st_mode & (S_IRUSR | S_IRGRP | S_IROTH))) return 403;
    if (S_ISDIR(info.st_mode) &&
        (!(info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) ||
        access(path.c_str(), X_OK) != 0)) return 403;
    return 200;
}
}

int PathResolver::normalizeTarget(const std::string& target,
    std::string& path, std::string& query) {
    path.clear();
    query.clear();
    if (target.empty() || target[0] != '/') return 400;
    for (size_t i = 0; i < target.size(); ++i)
        if (unsafeByte(static_cast<unsigned char>(target[i])) ||
            target[i] == '#') return 400;
    size_t mark = target.find('?');
    std::string raw = target.substr(0, mark);
    if (mark != std::string::npos) query = target.substr(mark);
    std::string decoded;
    for (size_t i = 0; i < raw.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(raw[i]);
        if (c == '%') {
            if (i + 2 >= raw.size()) return 400;
            int hi = hexDigit(raw[i + 1]), lo = hexDigit(raw[i + 2]);
            if (hi < 0 || lo < 0) return 400;
            c = static_cast<unsigned char>(hi * 16 + lo);
            i += 2;
        }
        if (unsafeByte(c)) return 400;
        decoded += static_cast<char>(c);
    }
    std::vector<std::string> parts;
    std::istringstream input(decoded);
    std::string part;
    bool trailing = decoded[decoded.size() - 1] == '/';
    while (std::getline(input, part, '/')) {
        if (part == "..") return 403;
        if (part == ".") { trailing = true; continue; }
        if (!part.empty()) { parts.push_back(part); trailing = false; }
    }
    trailing = trailing || decoded[decoded.size() - 1] == '/';
    path = "/";
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) path += "/";
        path += parts[i];
    }
    if (trailing && path != "/") path += "/";
    return 200;
}

std::string PathResolver::encodePath(const std::string& path) {
    const char* hex = "0123456789ABCDEF";
    std::string out;
    for (size_t i = 0; i < path.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(path[i]);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~' || c == '/') out += static_cast<char>(c);
        else { out += '%'; out += hex[c >> 4]; out += hex[c & 15]; }
    }
    return out;
}
std::string PathResolver::escapeHtml(const std::string& value) {
    std::string out;
    for (size_t i = 0; i < value.size(); ++i) {
        switch (value[i]) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        case '\'': out += "&#39;"; break;
        default: out += value[i];
        }
    }
    return out;
}
std::string PathResolver::descriptorPath(int fd) {
    std::ostringstream out;
    out << "/proc/self/fd/" << fd;
    return out.str();
}
int PathResolver::filesystemError(int error) {
    if (error == ENOENT || error == ENOTDIR) return 404;
    if (error == EACCES || error == EPERM || error == ELOOP) return 403;
    if (error == ENAMETOOLONG) return 414;
    return 500;
}
int PathResolver::openChild(int parent, const std::string& name,
    FileHandle& file, struct stat& info) {
    if (!safeComponent(name)) return 403;
    std::string path = descriptorPath(parent) + "/" + name;
    int fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) return filesystemError(errno);
    file.reset(fd);
    return inspect(fd, info);
}
int PathResolver::openBeneath(const std::string& base,
    const std::string& relative, FileHandle& file, struct stat& info) {
    // base is administrator configuration, never a value from the request.
    int fd = open(base.c_str(), O_RDONLY | O_DIRECTORY | O_NONBLOCK);
    if (fd < 0) return filesystemError(errno);
    file.reset(fd);
    int status = inspect(fd, info);
    if (status != 200) return status;
    std::istringstream input(relative);
    std::string part;
    while (std::getline(input, part, '/')) {
        if (part.empty()) continue;
        if (!safeComponent(part)) return 403;
        if (!S_ISDIR(info.st_mode)) return 404;
        std::string path = descriptorPath(file.get()) + "/" + part;
        fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
        if (fd < 0) return filesystemError(errno);
        file.reset(fd);
        status = inspect(fd, info);
        if (status != 200) return status;
    }
    return 200;
}
int PathResolver::readRegular(int fd, std::string& body) {
    struct stat info;
    int status = inspect(fd, info);
    if (status != 200) return status;
    if (!S_ISREG(info.st_mode)) return 403;
    body.clear();
    char buffer[16384];
    ssize_t count;
    while ((count = read(fd, buffer, sizeof(buffer))) > 0)
        body.append(buffer, static_cast<size_t>(count));
    // The subject forbids errno-based decisions after read/write.
    if (count < 0) { body.clear(); return 500; }
    return 200;
}