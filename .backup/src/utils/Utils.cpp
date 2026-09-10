#include "Utils.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <unistd.h>

namespace Utils {

std::string trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string> splitWhitespace(const std::string& str) {
	std::vector<std::string> tokens;
	std::istringstream iss(str);
	std::string token;
	while (iss >> token) {
		tokens.push_back(token);
	}
	return tokens;
}

std::string toLower(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i) {
		result[i] = std::tolower(static_cast<unsigned char>(result[i]));
	}
	return result;
}

std::string toUpper(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i) {
		result[i] = std::toupper(static_cast<unsigned char>(result[i]));
	}
	return result;
}

int toInt(const std::string& str) {
	return std::atoi(str.c_str());
}

size_t toSizeT(const std::string& str) {
	std::istringstream iss(str);
	size_t val = 0;
	iss >> val;
	return val;
}

size_t parseSize(const std::string& str) {
	if (str.empty()) return 0;
	std::string s = trim(str);
	char unit = s[s.length() - 1];
	size_t multiplier = 1;
	std::string numPart = s;

	if (unit == 'k' || unit == 'K') {
		multiplier = 1024;
		numPart = s.substr(0, s.length() - 1);
	} else if (unit == 'm' || unit == 'M') {
		multiplier = 1024 * 1024;
		numPart = s.substr(0, s.length() - 1);
	} else if (unit == 'g' || unit == 'G') {
		multiplier = 1024 * 1024 * 1024;
		numPart = s.substr(0, s.length() - 1);
	} else if (unit == 'b' || unit == 'B') {
		multiplier = 1;
		numPart = s.substr(0, s.length() - 1);
	}

	return toSizeT(numPart) * multiplier;
}

std::string getFileExtension(const std::string& path) {
	size_t lastDot = path.find_last_of('.');
	size_t lastSlash = path.find_last_of('/');
	if (lastDot != std::string::npos &&
	    (lastSlash == std::string::npos || lastDot > lastSlash)) {
		return path.substr(lastDot);
	}
	return "";
}

bool fileExists(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string& path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) return false;
	return S_ISDIR(buffer.st_mode);
}

bool isRegularFile(const std::string& path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) return false;
	return S_ISREG(buffer.st_mode);
}

std::string readFileToString(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) return "";
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::string normalizePath(const std::string& path) {
	std::vector<std::string> parts = split(path, '/');
	std::vector<std::string> cleanParts;

	for (size_t i = 0; i < parts.size(); ++i) {
		if (parts[i].empty() || parts[i] == ".") continue;
		if (parts[i] == "..") {
			if (!cleanParts.empty()) cleanParts.pop_back();
		} else {
			cleanParts.push_back(parts[i]);
		}
	}

	std::string result;
	if (!path.empty() && path[0] == '/') result += "/";
	for (size_t i = 0; i < cleanParts.size(); ++i) {
		result += cleanParts[i];
		if (i + 1 < cleanParts.size()) result += "/";
	}
	if (path.length() > 1 && path[path.length() - 1] == '/' &&
	    (result.empty() || result[result.length() - 1] != '/'))
		result += "/";
	return result.empty() ? "/" : result;
}

std::string urlDecode(const std::string& src) {
	std::string ret;
	char ch;
	int ii;
	for (size_t idx = 0; idx < src.length(); ++idx) {
		if (src[idx] == '%') {
			if (idx + 2 < src.length() && std::isxdigit(src[idx + 1]) &&
			    std::isxdigit(src[idx + 2])) {
				std::sscanf(src.substr(idx + 1, 2).c_str(), "%x", &ii);
				ch = static_cast<char>(ii);
				ret += ch;
				idx += 2;
			} else {
				ret += src[idx];
			}
		} else if (src[idx] == '+') {
			ret += ' ';
		} else {
			ret += src[idx];
		}
	}
	return ret;
}

std::string formatHttpDate(time_t t) {
	char buf[128];
	struct tm* tm_info = std::gmtime(&t);
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
	return std::string(buf);
}

std::string getMimeType(const std::string& path) {
	static std::map<std::string, std::string> mimeTypes;
	if (mimeTypes.empty()) {
		mimeTypes[".html"] = "text/html";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".json"] = "application/json";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".zip"] = "application/zip";
	}

	std::string ext = toLower(getFileExtension(path));
	std::map<std::string, std::string>::iterator it = mimeTypes.find(ext);
	if (it != mimeTypes.end()) return it->second;
	return "application/octet-stream";
}

}  // namespace Utils
