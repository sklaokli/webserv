#include "HttpResponse.hpp"
#include "Utils.hpp"
#include <ctime>
#include <sstream>

HttpResponse::HttpResponse() : m_statusCode(200), m_statusMessage("OK") {
	setHeader("Server", "Webserv/1.0");
	setHeader("Date", Utils::formatHttpDate(time(NULL)));
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatus(int statusCode, const std::string& statusMessage) {
	m_statusCode = statusCode;
	if (statusMessage.empty()) {
		m_statusMessage = getDefaultStatusMessage(statusCode);
	} else {
		m_statusMessage = statusMessage;
	}
}

int HttpResponse::getStatusCode() const {
	return m_statusCode;
}

const std::string& HttpResponse::getStatusMessage() const {
	return m_statusMessage;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
	m_headers[key] = value;
}

bool HttpResponse::hasHeader(const std::string& key) const {
	return m_headers.find(key) != m_headers.end();
}

std::string HttpResponse::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = m_headers.find(key);
	if (it != m_headers.end()) return it->second;
	return "";
}

void HttpResponse::setBody(const std::string& body) {
	m_body = body;
	setHeader("Content-Length", Utils::toString(m_body.length()));
}

const std::string& HttpResponse::getBody() const {
	return m_body;
}

void HttpResponse::setRedirect(int statusCode, const std::string& location) {
	setStatus(statusCode);
	setHeader("Location", location);
	setBody("<html><body><h1>" + Utils::toString(statusCode) + " " +
	        m_statusMessage + "</h1><p>The document has moved <a href=\"" +
	        location + "\">here</a>.</p></body></html>");
	setHeader("Content-Type", "text/html");
}

void HttpResponse::setCookie(const std::string& name, const std::string& value,
    const std::string& path, int maxAge, bool httpOnly) {
	std::ostringstream oss;
	oss << name << "=" << value << "; Path=" << path;
	if (maxAge >= 0) {
		oss << "; Max-Age=" << maxAge;
	}
	if (httpOnly) {
		oss << "; HttpOnly";
	}
	oss << "; SameSite=Lax";
	m_cookies.push_back(oss.str());
}

std::string HttpResponse::serialize() const {
	std::ostringstream oss;
	oss << "HTTP/1.1 " << m_statusCode << " " << m_statusMessage << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it =
	         m_headers.begin();
	     it != m_headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}

	for (size_t i = 0; i < m_cookies.size(); ++i) {
		oss << "Set-Cookie: " << m_cookies[i] << "\r\n";
	}

	oss << "\r\n";
	oss << m_body;
	return oss.str();
}

std::string HttpResponse::getDefaultStatusMessage(int statusCode) {
	switch (statusCode) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 204:
			return "No Content";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 304:
			return "Not Modified";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 408:
			return "Request Timeout";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		default:
			return "Unknown";
	}
}

std::string HttpResponse::generateDefaultErrorPage(
    int statusCode, const std::string& customMsg) {
	std::string msg =
	    customMsg.empty() ? getDefaultStatusMessage(statusCode) : customMsg;
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n"
	    << "<html lang=\"en\">\n"
	    << "<head>\n"
	    << "  <meta charset=\"UTF-8\">\n"
	    << "  <title>" << statusCode << " " << msg << "</title>\n"
	    << "  <style>\n"
	    << "    body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe "
	       "UI', Roboto, sans-serif; "
	    << "           text-align: center; padding: 100px; background-color: "
	       "#f8f9fa; color: #333; }\n"
	    << "    h1 { font-size: 50px; margin-bottom: 10px; color: #e63946; }\n"
	    << "    p { font-size: 20px; color: #6c757d; }\n"
	    << "    hr { width: 100px; border: 1px solid #dee2e6; margin: 20px "
	       "auto; }\n"
	    << "    footer { margin-top: 40px; font-size: 14px; color: #adb5bd; }\n"
	    << "  </style>\n"
	    << "</head>\n"
	    << "<body>\n"
	    << "  <h1>" << statusCode << "</h1>\n"
	    << "  <p>" << msg << "</p>\n"
	    << "  <hr>\n"
	    << "  <footer>webserv/1.0</footer>\n"
	    << "</body>\n"
	    << "</html>\n";
	return oss.str();
}
