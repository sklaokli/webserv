#include "http/MultipartParser.hpp"
#include <cctype>
#include <set>

UploadPart::UploadPart(const std::string& name, size_t start, size_t size)
    : filename(name), offset(start), length(size) {}

static std::string lowerText(std::string value) {
    for (size_t i = 0; i < value.size(); ++i)
        value[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[i])));
    return value;
}
static std::string trimText(const std::string& value) {
    size_t first = value.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    return value.substr(first, value.find_last_not_of(" \t") - first + 1);
}
static bool tokenByte(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || std::string("!#$%&'*+-.^_`|~").find(c) != std::string::npos;
}
static bool isToken(const std::string& value) {
    if (value.empty()) return false;
    for (size_t i = 0; i < value.size(); ++i)
        if (!tokenByte(static_cast<unsigned char>(value[i]))) return false;
    return true;
}
int MultipartParser::parseParameters(const std::string& header,
    std::string& type, std::map<std::string, std::string>& parameters) {
    parameters.clear();
    if (header.size() > 8192) return 400;
    for (size_t i = 0; i < header.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(header[i]);
        if ((c < 32 && c != '\t') || c == 127) return 400;
    }
    size_t pos = header.find(';');
    type = lowerText(trimText(header.substr(0, pos)));
    if (type.empty()) return 400;
    while (pos != std::string::npos && pos < header.size()) {
        ++pos;
        while (pos < header.size() && (header[pos] == ' ' || header[pos] == '\t')) ++pos;
        size_t start = pos;
        while (pos < header.size() && tokenByte(static_cast<unsigned char>(header[pos]))) ++pos;
        std::string key = lowerText(header.substr(start, pos - start));
        if (key.empty()) return 400;
        while (pos < header.size() && (header[pos] == ' ' || header[pos] == '\t')) ++pos;
        if (pos == header.size() || header[pos++] != '=') return 400;
        while (pos < header.size() && (header[pos] == ' ' || header[pos] == '\t')) ++pos;
        std::string value;
        if (pos < header.size() && header[pos] == '"') {
            ++pos;
            bool closed = false;
            while (pos < header.size()) {
                char c = header[pos++];
                if (c == '"') { closed = true; break; }
                if (c == '\\') {
                    if (pos == header.size()) return 400;
                    char next = header[pos];
                    // Strict quoted-pair: reject path-like ambiguous escapes.
                    if (next != '\\' && next != '"') return 400;
                    c = header[pos++];
                }
                value += c;
            }
            if (!closed) return 400;
        } else {
            start = pos;
            while (pos < header.size() && tokenByte(static_cast<unsigned char>(header[pos]))) ++pos;
            value = header.substr(start, pos - start);
            if (value.empty()) return 400;
        }
        while (pos < header.size() && (header[pos] == ' ' || header[pos] == '\t')) ++pos;
        if (pos < header.size() && header[pos] != ';') return 400;
        if (!parameters.insert(std::make_pair(key, value)).second) return 400;
    }
    return 200;
}
bool MultipartParser::validBoundary(const std::string& boundary) {
    if (boundary.empty() || boundary.size() > 70 || boundary[boundary.size() - 1] == ' ')
        return false;
    for (size_t i = 0; i < boundary.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(boundary[i]);
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || std::string("'()+_,-./:=? ").find(c) != std::string::npos))
            return false;
    }
    return true;
}
bool MultipartParser::validFilename(const std::string& filename) {
    if (filename.empty() || filename.size() > 255 || filename == "." || filename == "..")
        return false;
    if (filename[filename.size() - 1] == '.' || filename[filename.size() - 1] == ' ')
        return false;
    for (size_t i = 0; i < filename.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(filename[i]);
        if (c < 32 || c == 127 || std::string("/\\:\"<>|?*").find(c) != std::string::npos)
            return false;
    }
    return true;
}
static bool delimiterEnd(const std::string& body, size_t after, bool& final, size_t& next) {
    final = body.compare(after, 2, "--") == 0;
    if (final) after += 2;
    while (after < body.size() && (body[after] == ' ' || body[after] == '\t')) ++after;
    if (final && after == body.size()) { next = after; return true; }
    if (body.compare(after, 2, "\r\n") != 0) return false;
    next = after + 2;
    return true;
}
static int parsePartHeaders(const std::string& block, std::string& filename, bool& hasFile) {
    std::map<std::string, std::string> headers;
    size_t pos = 0;
    while (pos < block.size()) {
        size_t end = block.find("\r\n", pos);
        if (end == std::string::npos) end = block.size();
        std::string line = block.substr(pos, end - pos);
        size_t colon = line.find(':');
        if (colon == std::string::npos || !isToken(line.substr(0, colon))) return 400;
        std::string key = lowerText(line.substr(0, colon));
        std::string value = trimText(line.substr(colon + 1));
        for (size_t i = 0; i < value.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(value[i]);
            if ((c < 32 && c != '\t') || c == 127) return 400;
        }
        if (!headers.insert(std::make_pair(key, value)).second) return 400;
        pos = end == block.size() ? end : end + 2;
    }
    if (!headers.count("content-disposition")) return 400;
    if (headers.count("content-transfer-encoding")) return 415;
    std::string disposition;
    std::map<std::string, std::string> params;
    int status = MultipartParser::parseParameters(headers["content-disposition"], disposition, params);
    if (status != 200 || disposition != "form-data" || !params.count("name") ||
        params["name"].empty() || params.count("filename*")) return 400;
    hasFile = params.count("filename") && !params["filename"].empty();
    filename = hasFile ? params["filename"] : "";
    return hasFile && !MultipartParser::validFilename(filename) ? 403 : 200;
}
int MultipartParser::parse(const std::string& body, const std::string& boundary,
    std::vector<UploadPart>& files) {
    files.clear();
    if (!validBoundary(boundary)) return 400;
    const std::string marker = "--" + boundary;
    size_t pos = body.find(marker);
    bool final = false;
    size_t next = 0;
    while (pos != std::string::npos) {
        if ((pos == 0 || (pos >= 2 && body.compare(pos - 2, 2, "\r\n") == 0)) &&
            delimiterEnd(body, pos + marker.size(), final, next)) break;
        pos = body.find(marker, pos + marker.size());
    }
    if (pos == std::string::npos || final) return 400;
    pos = next;
    size_t partCount = 0;
    std::set<std::string> names;
    while (true) {
        if (++partCount > 128) return 400;
        size_t headerEnd = body.find("\r\n\r\n", pos);
        if (headerEnd == std::string::npos || headerEnd - pos > 16384) return 400;
        std::string filename;
        bool hasFile;
        int status = parsePartHeaders(body.substr(pos, headerEnd - pos), filename, hasFile);
        if (status != 200) return status;
        size_t dataStart = headerEnd + 4;
        const std::string delimiter = "\r\n" + marker;
        size_t dataEnd = body.find(delimiter, dataStart);
        while (dataEnd != std::string::npos &&
            !delimiterEnd(body, dataEnd + delimiter.size(), final, next))
            dataEnd = body.find(delimiter, dataEnd + delimiter.size());
        if (dataEnd == std::string::npos) return 400;
        if (hasFile) {
            if (!names.insert(filename).second) return 409;
            files.push_back(UploadPart(filename, dataStart, dataEnd - dataStart));
        }
        if (final) break;
        pos = next;
    }
    return files.empty() ? 400 : 200;
}
