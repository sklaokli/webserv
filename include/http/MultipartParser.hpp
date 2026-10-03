#ifndef MULTIPARTPARSER_HPP
#define MULTIPARTPARSER_HPP

#include <map>
#include <string>
#include <vector>

// Offsets refer to the original body: binary file data is never decoded.
struct UploadPart {
    std::string filename;
    size_t offset;
    size_t length;
    UploadPart(const std::string& name, size_t start, size_t size);
};

class MultipartParser {
public:
    static int parseParameters(const std::string& header, std::string& type,
        std::map<std::string, std::string>& parameters);
    static int parse(const std::string& body, const std::string& boundary,
        std::vector<UploadPart>& files);
    static bool validBoundary(const std::string& boundary);
    static bool validFilename(const std::string& filename);
};
#endif
