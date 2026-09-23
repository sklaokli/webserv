#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP
#include <string>
class MimeTypes {
public:
    static std::string lookup(const std::string& path);
};
#endif