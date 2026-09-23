#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP
#include <string>
class AutoIndex {
public:
    static int generate(int directoryFd, const std::string& urlPath,
        std::string& html);
};
#endif