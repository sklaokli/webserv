#include "http/AutoIndex.hpp"
#include "http/PathResolver.hpp"
#include <algorithm>
#include <cerrno>
#include <ctime>
#include <dirent.h>
#include <iomanip>
#include <sstream>
#include <vector>
namespace {
struct DirectoryHandle {
    DIR* value;
    explicit DirectoryHandle(DIR* p) : value(p) {}
    ~DirectoryHandle() { if (value) closedir(value); }
private:
    DirectoryHandle(const DirectoryHandle&);
    DirectoryHandle& operator=(const DirectoryHandle&);
};
struct Entry {
    std::string name;
    struct stat info;
};
bool lessEntry(const Entry& a, const Entry& b) {
    bool ad = S_ISDIR(a.info.st_mode), bd = S_ISDIR(b.info.st_mode);
    if (ad != bd) return ad;
    return a.name < b.name;
}
std::string sizeText(const Entry& entry) {
    if (S_ISDIR(entry.info.st_mode)) return "-";
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    double size = static_cast<double>(entry.info.st_size);
    size_t unit = 0;
    while (size >= 1024.0 && unit < 4) { size /= 1024.0; ++unit; }
    std::ostringstream out;
    out << std::fixed << std::setprecision(unit ? 1 : 0) << size << " " << units[unit];
    return out.str();
}
std::string dateText(std::time_t time) {
    char buffer[32];
    std::tm* utc = std::gmtime(&time);
    if (!utc || !std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", utc))
        return "-";
    return buffer;
}
}
int AutoIndex::generate(int directoryFd, const std::string& urlPath,
    std::string& html) {
    DirectoryHandle directory(opendir(PathResolver::descriptorPath(directoryFd).c_str()));
    if (!directory.value) return PathResolver::filesystemError(errno);
    std::vector<Entry> entries;
    while (true) {
        errno = 0;
        struct dirent* item = readdir(directory.value);
        if (!item) {
            if (errno) return 500;
            break;
        }
        Entry entry;
        entry.name = item->d_name;
        if (entry.name == "." || entry.name == "..") continue;
        FileHandle child;
        int status = PathResolver::openChild(directoryFd, entry.name, child, entry.info);
        // Broken links, symlinks, special files and inaccessible entries are omitted.
        if (status == 403 || status == 404 || status == 414) continue;
        if (status != 200) return status;
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), lessEntry);
    const std::string title = "Index of " + PathResolver::escapeHtml(urlPath);
    std::ostringstream out;
    out << "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
        << "<title>" << title << "</title></head><body><h1>" << title
        << "</h1><table><thead><tr><th>Name</th><th>Last modified</th><th>Size</th>"
        << "</tr></thead><tbody>\n";
    if (urlPath != "/") {
        size_t slash = urlPath.find_last_of('/', urlPath.size() - 2);
        std::string parent = urlPath.substr(0, slash + 1);
        out << "<tr><td><a href=\"" << PathResolver::encodePath(parent)
            << "\">../</a></td><td>-</td><td>-</td></tr>\n";
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        std::string name = entries[i].name;
        if (S_ISDIR(entries[i].info.st_mode)) name += "/";
        out << "<tr><td><a href=\"" << PathResolver::encodePath(urlPath + name)
            << "\">" << PathResolver::escapeHtml(name) << "</a></td><td>"
            << dateText(entries[i].info.st_mtime) << "</td><td>"
            << sizeText(entries[i]) << "</td></tr>\n";
    }
    out << "</tbody></table></body></html>\n";
    html = out.str();
    return 200;
}