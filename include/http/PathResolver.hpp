#ifndef PATHRESOLVER_HPP
#define PATHRESOLVER_HPP
#include <string>
#include <sys/stat.h>

// Owns a descriptor; deliberately noncopyable (C++98).
class FileHandle {
public:
    explicit FileHandle(int fd = -1);
    ~FileHandle();
    int get() const;
    void reset(int fd);
private:
    FileHandle(const FileHandle&);
    FileHandle& operator=(const FileHandle&);
    int _fd;
};

class PathResolver {
public:
    // Decode exactly once. Reject all ".." components before routing.
    static int normalizeTarget(const std::string& target,
        std::string& path, std::string& query);
    static std::string encodePath(const std::string& path);
    static std::string escapeHtml(const std::string& value);
    // Linux/WSL: open relative to pinned directory descriptors through procfs.
    // Symlinks below the trusted configured base are never followed.
    static int openBeneath(const std::string& base, const std::string& relative,
        FileHandle& file, struct stat& info);
    static int openChild(int parent, const std::string& name,
        FileHandle& file, struct stat& info);
    static std::string descriptorPath(int fd);
    static int readRegular(int fd, std::string& body);
    static int filesystemError(int error);
};
#endif