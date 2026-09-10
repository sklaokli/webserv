#include "AutoIndex.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <dirent.h>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <vector>

namespace AutoIndex {

struct EntryInfo {
	std::string name;
	bool isDir;
	std::string date;
	std::string sizeStr;
};

bool compareEntries(const EntryInfo& a, const EntryInfo& b) {
	if (a.isDir && !b.isDir) return true;
	if (!a.isDir && b.isDir) return false;
	return a.name < b.name;
}

std::string generatePage(
    const std::string& dirPath, const std::string& requestUri) {
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) return "";

	std::vector<EntryInfo> entries;
	struct dirent* ent;

	while ((ent = readdir(dir)) != NULL) {
		std::string name = ent->d_name;
		if (name == ".") continue;

		EntryInfo info;
		info.name = name;

		std::string fullPath = dirPath;
		if (fullPath[fullPath.length() - 1] != '/') fullPath += "/";
		fullPath += name;

		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0) {
			info.isDir = S_ISDIR(st.st_mode);

			char dateBuf[32];
			struct tm* t = localtime(&st.st_mtime);
			strftime(dateBuf, sizeof(dateBuf), "%d-%b-%Y %H:%M", t);
			info.date = dateBuf;

			if (info.isDir) {
				info.sizeStr = "-";
			} else {
				info.sizeStr = Utils::toString(st.st_size);
			}
		} else {
			info.isDir = false;
			info.date = "-";
			info.sizeStr = "-";
		}

		entries.push_back(info);
	}
	closedir(dir);

	std::sort(entries.begin(), entries.end(), compareEntries);

	std::string cleanUri = requestUri;
	if (cleanUri.empty() || cleanUri[cleanUri.length() - 1] != '/')
		cleanUri += "/";

	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n"
	    << "<html>\n"
	    << "<head>\n"
	    << "  <title>Index of " << cleanUri << "</title>\n"
	    << "  <style>\n"
	    << "    body { font-family: monospace; padding: 20px; background: "
	       "#fff; color: #222; }\n"
	    << "    h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
	    << "    table { width: 100%; max-width: 900px; border-collapse: "
	       "collapse; }\n"
	    << "    th, td { text-align: left; padding: 6px 12px; }\n"
	    << "    tr:nth-child(even) { background-color: #f8f9fa; }\n"
	    << "    a { color: #0366d6; text-decoration: none; }\n"
	    << "    a:hover { text-decoration: underline; }\n"
	    << "    .size { text-align: right; }\n"
	    << "  </style>\n"
	    << "</head>\n"
	    << "<body>\n"
	    << "  <h1>Index of " << cleanUri << "</h1>\n"
	    << "  <table>\n"
	    << "    <tr><th>Name</th><th>Last modified</th><th "
	       "class=\"size\">Size</th></tr>\n";

	if (cleanUri != "/") {
		oss << "    <tr><td><a href=\"..\">../</a></td><td>-</td><td "
		       "class=\"size\">-</td></tr>\n";
	}

	for (size_t i = 0; i < entries.size(); ++i) {
		std::string displayName =
		    entries[i].name + (entries[i].isDir ? "/" : "");
		std::string href = displayName;
		oss << "    <tr>"
		    << "<td><a href=\"" << href << "\">" << displayName << "</a></td>"
		    << "<td>" << entries[i].date << "</td>"
		    << "<td class=\"size\">" << entries[i].sizeStr << "</td>"
		    << "</tr>\n";
	}

	oss << "  </table>\n"
	    << "  <hr>\n"
	    << "  <address>webserv/1.0</address>\n"
	    << "</body>\n"
	    << "</html>\n";

	return oss.str();
}

}  // namespace AutoIndex
