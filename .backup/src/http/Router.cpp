#include "Router.hpp"
#include "AutoIndex.hpp"
#include "Logger.hpp"
#include "SessionManager.hpp"
#include "Utils.hpp"
#include <ctime>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

Router::Router() {}
Router::~Router() {}

std::string Router::resolvePath(
    const std::string& uri, const LocationConfig& loc) {
	std::string rel;
	if (uri.find(loc.path) == 0) {
		rel = uri.substr(loc.path.length());
	} else {
		rel = uri;
	}

	if (!rel.empty() && rel[0] == '/') {
		rel.erase(0, 1);
	}

	std::string base = loc.root;
	if (base.empty()) base = ".";
	if (base[base.length() - 1] != '/') base += "/";

	return base + rel;
}

HttpResponse Router::buildErrorResponse(
    int statusCode, const ServerConfig& server, const std::string& customMsg) {
	HttpResponse res;
	res.setStatus(statusCode);
	res.setHeader("Connection", "close");

	std::string errorPage = server.getErrorPage(statusCode);
	if (!errorPage.empty()) {
		std::string fullErrorPath = server.root;
		if (!fullErrorPath.empty() &&
		    fullErrorPath[fullErrorPath.length() - 1] != '/' &&
		    errorPage[0] != '/')
			fullErrorPath += "/";
		fullErrorPath += errorPage;

		if (Utils::isRegularFile(fullErrorPath)) {
			std::string content = Utils::readFileToString(fullErrorPath);
			if (!content.empty()) {
				res.setHeader("Content-Type", "text/html");
				res.setBody(content);
				return res;
			}
		}
	}

	res.setHeader("Content-Type", "text/html");
	res.setBody(HttpResponse::generateDefaultErrorPage(statusCode, customMsg));
	return res;
}

bool Router::isCgiRequest(const HttpRequest& request, const LocationConfig* loc,
    std::string& scriptPath, std::string& interpreter) {
	if (!loc) return false;

	std::string path = request.getPath();
	std::string ext = Utils::getFileExtension(path);
	if (ext.empty()) return false;

	if (loc->hasCgiExtension(ext)) {
		interpreter = loc->getCgiInterpreter(ext);
		scriptPath = resolvePath(path, *loc);
		return true;
	}

	return false;
}

HttpResponse Router::handleRequest(
    const HttpRequest& request, const ServerConfig& server) {
	if (request.hasError()) {
		return buildErrorResponse(request.getErrorCode(), server);
	}

	// Check Session & Cookie API Endpoints (Bonus feature)
	HttpResponse sessionRes;
	if (handleSessionApi(request, sessionRes)) {
		return sessionRes;
	}

	const LocationConfig* loc = server.findLocation(request.getPath());
	if (!loc) {
		return buildErrorResponse(404, server);
	}

	// Check redirection
	if (loc->redirect_code > 0 && !loc->redirect_url.empty()) {
		HttpResponse res;
		res.setRedirect(loc->redirect_code, loc->redirect_url);
		return res;
	}

	// Check allowed methods
	if (!loc->isMethodAllowed(request.getMethod())) {
		HttpResponse res = buildErrorResponse(405, server);
		std::string allowHeader;
		for (std::set<std::string>::const_iterator it =
		         loc->allowed_methods.begin();
		     it != loc->allowed_methods.end(); ++it) {
			if (!allowHeader.empty()) allowHeader += ", ";
			allowHeader += *it;
		}
		res.setHeader("Allow", allowHeader);
		return res;
	}

	size_t maxBody = loc->client_max_body_size > 0
	                     ? loc->client_max_body_size
	                     : server.client_max_body_size;
	if (maxBody > 0 && request.getBody().length() > maxBody) {
		return buildErrorResponse(413, server);
	}

	std::string fullPath = resolvePath(request.getPath(), *loc);

	if (request.getMethod() == "GET" || request.getMethod() == "HEAD") {
		return handleGet(request, *loc, server, fullPath);
	} else if (request.getMethod() == "POST") {
		return handlePost(request, *loc, server, fullPath);
	} else if (request.getMethod() == "DELETE") {
		return handleDelete(request, *loc, server, fullPath);
	} else {
		return buildErrorResponse(501, server, "Not Implemented");
	}
}

HttpResponse Router::handleGet(const HttpRequest& request,
    const LocationConfig& loc, const ServerConfig& server,
    const std::string& fullPath) {
	if (!Utils::fileExists(fullPath)) {
		return buildErrorResponse(404, server);
	}

	if (Utils::isDirectory(fullPath)) {
		// Enforce trailing slash on directory URLs
		std::string uri = request.getPath();
		if (uri.empty() || uri[uri.length() - 1] != '/') {
			HttpResponse res;
			res.setRedirect(301, uri + "/");
			return res;
		}

		// Check index file
		std::string indexPath = fullPath;
		if (indexPath[indexPath.length() - 1] != '/') indexPath += "/";
		indexPath += loc.index;

		if (Utils::isRegularFile(indexPath)) {
			if (access(indexPath.c_str(), R_OK) != 0) {
				return buildErrorResponse(403, server);
			}
			HttpResponse res;
			res.setStatus(200);
			res.setHeader("Content-Type", Utils::getMimeType(indexPath));
			res.setBody(Utils::readFileToString(indexPath));
			return res;
		}

		// Check autoindex
		if (loc.autoindex) {
			std::string listing = AutoIndex::generatePage(fullPath, uri);
			if (!listing.empty()) {
				HttpResponse res;
				res.setStatus(200);
				res.setHeader("Content-Type", "text/html");
				res.setBody(listing);
				return res;
			}
		}

		return buildErrorResponse(403, server);
	}

	if (Utils::isRegularFile(fullPath)) {
		if (access(fullPath.c_str(), R_OK) != 0) {
			return buildErrorResponse(403, server);
		}

		HttpResponse res;
		res.setStatus(200);
		res.setHeader("Content-Type", Utils::getMimeType(fullPath));
		res.setBody(Utils::readFileToString(fullPath));
		return res;
	}

	return buildErrorResponse(403, server);
}

HttpResponse Router::handlePost(const HttpRequest& request,
    const LocationConfig& loc, const ServerConfig& server,
    const std::string& fullPath) {
	if (!loc.upload_enable) {
		return buildErrorResponse(403, server, "Upload Not Allowed");
	}

	std::string uploadDir =
	    loc.upload_store.empty() ? loc.root : loc.upload_store;
	if (!Utils::isDirectory(uploadDir)) {
		return buildErrorResponse(500, server, "Upload Directory Not Found");
	}

	std::string filename;
	std::string fileContent;
	std::string contentType = request.getHeader("content-type");

	if (contentType.find("multipart/form-data") != std::string::npos) {
		// Extract boundary
		size_t bPos = contentType.find("boundary=");
		if (bPos == std::string::npos) {
			return buildErrorResponse(
			    400, server, "Missing Boundary in Multipart Form");
		}
		std::string boundary = "--" + contentType.substr(bPos + 9);
		// Remove quotes if any
		if (boundary.length() > 2 && boundary[2] == '"') {
			boundary.erase(2, 1);
			if (boundary[boundary.length() - 1] == '"')
				boundary.erase(boundary.length() - 1, 1);
		}

		const std::string& body = request.getBody();
		size_t partStart = body.find(boundary);
		if (partStart != std::string::npos) {
			size_t nextBoundary =
			    body.find(boundary, partStart + boundary.length());
			if (nextBoundary != std::string::npos) {
				std::string part = body.substr(partStart + boundary.length(),
				    nextBoundary - (partStart + boundary.length()));
				size_t headerEnd = part.find("\r\n\r\n");
				if (headerEnd != std::string::npos) {
					std::string partHeaders = part.substr(0, headerEnd);
					fileContent = part.substr(headerEnd + 4);
					// Trim trailing \r\n before boundary
					if (fileContent.length() >= 2 &&
					    fileContent.substr(fileContent.length() - 2) == "\r\n")
						fileContent.erase(fileContent.length() - 2);

					// Extract filename="abc.xyz"
					size_t fnPos = partHeaders.find("filename=\"");
					if (fnPos != std::string::npos) {
						size_t fnEnd = partHeaders.find("\"", fnPos + 10);
						if (fnEnd != std::string::npos) {
							filename = partHeaders.substr(
							    fnPos + 10, fnEnd - (fnPos + 10));
						}
					}
				}
			}
		}
	}

	// If filename wasn't found in multipart, look at URI or generate fallback
	if (filename.empty()) {
		size_t slash = fullPath.find_last_of('/');
		if (slash != std::string::npos && slash + 1 < fullPath.length()) {
			filename = fullPath.substr(slash + 1);
		} else {
			filename = "upload_" + Utils::toString(time(NULL)) + ".bin";
		}
		fileContent = request.getBody();
	}

	// Sanitize filename to prevent traversal
	size_t lastSlash = filename.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		filename = filename.substr(lastSlash + 1);
	if (filename.empty())
		filename = "upload_" + Utils::toString(time(NULL)) + ".bin";

	std::string destPath = uploadDir;
	if (destPath[destPath.length() - 1] != '/') destPath += "/";
	destPath += filename;

	std::ofstream outFile(destPath.c_str(), std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		return buildErrorResponse(500, server, "Cannot Save Uploaded File");
	}
	outFile.write(fileContent.c_str(), fileContent.length());
	outFile.close();

	HttpResponse res;
	res.setStatus(201, "Created");
	res.setHeader("Location",
	    loc.path + (loc.path[loc.path.length() - 1] == '/' ? "" : "/") +
	        filename);
	res.setHeader("Content-Type", "text/html");
	res.setBody(
	    "<html><body><h1>File uploaded successfully!</h1><p><a href=\"" +
	    loc.path + (loc.path[loc.path.length() - 1] == '/' ? "" : "/") +
	    filename + "\">View " + filename + "</a></p></body></html>");
	return res;
}

HttpResponse Router::handleDelete(const HttpRequest& request,
    const LocationConfig& loc, const ServerConfig& server,
    const std::string& fullPath) {
	(void)request;
	(void)loc;

	if (!Utils::fileExists(fullPath)) {
		return buildErrorResponse(404, server);
	}

	if (Utils::isDirectory(fullPath)) {
		return buildErrorResponse(403, server, "Cannot Delete Directory");
	}

	if (unlink(fullPath.c_str()) != 0) {
		return buildErrorResponse(500, server, "Failed to Delete File");
	}

	HttpResponse res;
	res.setStatus(204, "No Content");
	return res;
}

bool Router::handleSessionApi(
    const HttpRequest& request, HttpResponse& response) {
	const std::string& path = request.getPath();
	const std::string& method = request.getMethod();

	if (path == "/api/session") {
		if (method != "GET") {
			response.setStatus(405);
			response.setHeader("Allow", "GET");
			response.setHeader("Content-Type", "application/json");
			response.setBody("{\"error\":\"Method Not Allowed\"}");
			return true;
		}

		std::string sessId = request.getCookie("session_id");
		Session* session = SessionManager::getSession(sessId);
		response.setStatus(200);
		response.setHeader("Content-Type", "application/json");

		if (session) {
			std::ostringstream oss;
			oss << "{\"active\":true,"
			    << "\"sessionId\":\"" << session->id << "\","
			    << "\"username\":\"" << session->username << "\","
			    << "\"visits\":" << session->visitCount << "}";
			response.setBody(oss.str());
		} else {
			response.setBody(
			    "{\"active\":false,\"message\":\"No active session\"}");
		}
		return true;
	} else if (path == "/api/session/login") {
		if (method != "POST") {
			response.setStatus(405);
			response.setHeader("Allow", "POST");
			response.setHeader("Content-Type", "application/json");
			response.setBody("{\"error\":\"Method Not Allowed\"}");
			return true;
		}

		std::string username = "42Student";
		const std::string& body = request.getBody();
		size_t uPos = body.find("username=");
		if (uPos != std::string::npos) {
			size_t endPos = body.find('&', uPos + 9);
			if (endPos != std::string::npos) {
				username = body.substr(uPos + 9, endPos - (uPos + 9));
			} else {
				username = body.substr(uPos + 9);
			}
			username = Utils::urlDecode(username);
		} else {
			size_t jPos = body.find("\"username\"");
			if (jPos != std::string::npos) {
				size_t q1 = body.find('"', jPos + 10);
				if (q1 != std::string::npos) {
					size_t q2 = body.find('"', q1 + 1);
					if (q2 != std::string::npos) {
						username = body.substr(q1 + 1, q2 - (q1 + 1));
					}
				}
			}
		}

		if (username.empty()) username = "42Student";

		Session* session = SessionManager::createSession(username);
		response.setStatus(200);
		response.setHeader("Content-Type", "application/json");
		response.setCookie("session_id", session->id, "/", 3600, true);

		std::ostringstream oss;
		oss << "{\"success\":true,"
		    << "\"sessionId\":\"" << session->id << "\","
		    << "\"username\":\"" << session->username << "\","
		    << "\"visits\":" << session->visitCount << "}";
		response.setBody(oss.str());
		return true;
	} else if (path == "/api/session/logout") {
		if (method != "POST") {
			response.setStatus(405);
			response.setHeader("Allow", "POST");
			response.setHeader("Content-Type", "application/json");
			response.setBody("{\"error\":\"Method Not Allowed\"}");
			return true;
		}

		std::string sessId = request.getCookie("session_id");
		if (!sessId.empty()) {
			SessionManager::destroySession(sessId);
		}

		response.setStatus(200);
		response.setHeader("Content-Type", "application/json");
		response.setCookie("session_id", "", "/", 0, true);
		response.setBody(
		    "{\"success\":true,\"message\":\"Logged out successfully\"}");
		return true;
	}

	return false;
}
