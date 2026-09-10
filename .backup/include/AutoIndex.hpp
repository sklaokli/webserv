#ifndef AUTO_INDEX_HPP
#define AUTO_INDEX_HPP

#include <string>

namespace AutoIndex {
// Generates an HTML directory listing page for a given directory path and
// request URI. Returns empty string on error (e.g. opendir fails).
std::string generatePage(
    const std::string& dirPath, const std::string& requestUri);
}  // namespace AutoIndex

#endif
