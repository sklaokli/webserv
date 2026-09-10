#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace Utils {
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> splitWhitespace(const std::string& str);
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);

template <typename T>
std::string toString(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

int toInt(const std::string& str);
size_t toSizeT(const std::string& str);
size_t parseSize(const std::string& str);

std::string getFileExtension(const std::string& path);
bool fileExists(const std::string& path);
bool isDirectory(const std::string& path);
bool isRegularFile(const std::string& path);
std::string readFileToString(const std::string& path);

std::string normalizePath(const std::string& path);
std::string urlDecode(const std::string& src);
std::string formatHttpDate(time_t t);
std::string getMimeType(const std::string& path);
}  // namespace Utils

#endif
