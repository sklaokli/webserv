#include "HttpRequest.hpp"
#include "Utils.hpp"
#include <cstdlib>
#include <sstream>

HttpRequest::HttpRequest()
    : m_state(STATE_REQUEST_LINE)
    , m_errorCode(0)
    , m_contentLength(0)
    , m_isChunked(false)
    , m_chunkSize(0) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::reset() {
	m_state = STATE_REQUEST_LINE;
	m_errorCode = 0;
	m_rawBuffer.clear();
	m_method.clear();
	m_uri.clear();
	m_path.clear();
	m_queryString.clear();
	m_version.clear();
	m_headers.clear();
	m_cookies.clear();
	m_body.clear();
	m_contentLength = 0;
	m_isChunked = false;
	m_chunkSize = 0;
}

const std::string& HttpRequest::getMethod() const {
	return m_method;
}
const std::string& HttpRequest::getUri() const {
	return m_uri;
}
const std::string& HttpRequest::getPath() const {
	return m_path;
}
const std::string& HttpRequest::getQueryString() const {
	return m_queryString;
}
const std::string& HttpRequest::getVersion() const {
	return m_version;
}
const std::string& HttpRequest::getBody() const {
	return m_body;
}
const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
	return m_headers;
}

bool HttpRequest::hasHeader(const std::string& name) const {
	return m_headers.find(Utils::toLower(name)) != m_headers.end();
}

std::string HttpRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it =
	    m_headers.find(Utils::toLower(name));
	if (it != m_headers.end()) return it->second;
	return "";
}

bool HttpRequest::isComplete() const {
	return m_state == STATE_COMPLETE;
}

bool HttpRequest::hasError() const {
	return m_state == STATE_ERROR;
}

int HttpRequest::getErrorCode() const {
	return m_errorCode;
}

bool HttpRequest::isKeepAlive() const {
	std::string conn = getHeader("connection");
	if (m_version == "HTTP/1.1") {
		return (conn != "close");
	} else if (m_version == "HTTP/1.0") {
		return (conn == "keep-alive");
	}
	return false;
}

void HttpRequest::parseUri() {
	size_t qPos = m_uri.find('?');
	if (qPos != std::string::npos) {
		m_path = m_uri.substr(0, qPos);
		m_queryString = m_uri.substr(qPos + 1);
	} else {
		m_path = m_uri;
		m_queryString.clear();
	}
	m_path = Utils::urlDecode(m_path);
	m_path = Utils::normalizePath(m_path);
}

bool HttpRequest::parseRequestLine(const std::string& line) {
	std::vector<std::string> parts = Utils::splitWhitespace(line);
	if (parts.size() != 3) {
		m_errorCode = 400;  // Bad Request
		m_state = STATE_ERROR;
		return false;
	}

	m_method = parts[0];
	m_uri = parts[1];
	m_version = parts[2];

	parseUri();

	if (m_version != "HTTP/1.0" && m_version != "HTTP/1.1") {
		m_errorCode = 505;  // HTTP Version Not Supported
		m_state = STATE_ERROR;
		return false;
	}

	return true;
}

bool HttpRequest::parseHeaderLine(const std::string& line) {
	size_t colon = line.find(':');
	if (colon == std::string::npos) {
		m_errorCode = 400;
		m_state = STATE_ERROR;
		return false;
	}

	std::string key = Utils::toLower(Utils::trim(line.substr(0, colon)));
	std::string value = Utils::trim(line.substr(colon + 1));
	m_headers[key] = value;

	if (key == "cookie") {
		parseCookies(value);
	}

	return true;
}

void HttpRequest::parseCookies(const std::string& cookieHeader) {
	std::vector<std::string> pairs = Utils::split(cookieHeader, ';');
	for (size_t i = 0; i < pairs.size(); ++i) {
		std::string pair = Utils::trim(pairs[i]);
		size_t eq = pair.find('=');
		if (eq != std::string::npos) {
			std::string name = Utils::trim(pair.substr(0, eq));
			std::string val = Utils::trim(pair.substr(eq + 1));
			if (!name.empty()) {
				m_cookies[name] = val;
			}
		}
	}
}

bool HttpRequest::hasCookie(const std::string& name) const {
	return m_cookies.find(name) != m_cookies.end();
}

std::string HttpRequest::getCookie(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it =
	    m_cookies.find(name);
	if (it != m_cookies.end()) return it->second;
	return "";
}

const std::map<std::string, std::string>& HttpRequest::getCookies() const {
	return m_cookies;
}

bool HttpRequest::parseChunk(const std::string& data, size_t maxBodySize) {
	m_rawBuffer += data;

	while (m_state != STATE_COMPLETE && m_state != STATE_ERROR) {
		if (m_state == STATE_REQUEST_LINE) {
			size_t crlf = m_rawBuffer.find("\r\n");
			if (crlf == std::string::npos) break;  // need more data

			std::string line = m_rawBuffer.substr(0, crlf);
			m_rawBuffer.erase(0, crlf + 2);

			if (line.empty())  // ignore empty lines before request
				continue;

			if (!parseRequestLine(line)) return true;

			m_state = STATE_HEADERS;
		} else if (m_state == STATE_HEADERS) {
			size_t crlf = m_rawBuffer.find("\r\n");
			if (crlf == std::string::npos) break;  // need more data

			std::string line = m_rawBuffer.substr(0, crlf);
			m_rawBuffer.erase(0, crlf + 2);

			if (line.empty()) {
				// RFC 7230 §5.4: HTTP/1.1 requests MUST include a Host header
				if (m_version == "HTTP/1.1" && !hasHeader("host")) {
					m_errorCode = 400;
					m_state = STATE_ERROR;
					return true;
				}

				// End of headers! Determine body parsing mode
				if (hasHeader("transfer-encoding") &&
				    getHeader("transfer-encoding").find("chunked") !=
				        std::string::npos) {
					m_isChunked = true;
					m_state = STATE_CHUNKED_SIZE;
				} else if (hasHeader("content-length")) {
					m_contentLength =
					    Utils::toSizeT(getHeader("content-length"));
					if (maxBodySize > 0 && m_contentLength > maxBodySize) {
						m_errorCode = 413;  // Payload Too Large
						m_state = STATE_ERROR;
						return true;
					}
					if (m_contentLength == 0) {
						m_state = STATE_COMPLETE;
						return true;
					}
					m_state = STATE_BODY;
				} else {
					// No body
					m_state = STATE_COMPLETE;
					return true;
				}
			} else {
				if (!parseHeaderLine(line)) return true;
			}
		} else if (m_state == STATE_BODY) {
			size_t needed = m_contentLength - m_body.length();
			size_t available = m_rawBuffer.length();
			size_t toTake = (available < needed) ? available : needed;

			m_body.append(m_rawBuffer, 0, toTake);
			m_rawBuffer.erase(0, toTake);

			if (maxBodySize > 0 && m_body.length() > maxBodySize) {
				m_errorCode = 413;
				m_state = STATE_ERROR;
				return true;
			}

			if (m_body.length() >= m_contentLength) {
				m_state = STATE_COMPLETE;
				return true;
			}
			break;  // need more data
		} else if (m_state == STATE_CHUNKED_SIZE) {
			size_t crlf = m_rawBuffer.find("\r\n");
			if (crlf == std::string::npos) break;  // need more data

			std::string line = m_rawBuffer.substr(0, crlf);
			m_rawBuffer.erase(0, crlf + 2);

			// Strip chunk extensions if any (e.g. "1a;ext=foo")
			size_t semi = line.find(';');
			if (semi != std::string::npos) line = line.substr(0, semi);
			line = Utils::trim(line);

			char* endPtr = NULL;
			long sizeVal = std::strtol(line.c_str(), &endPtr, 16);
			if (endPtr == line.c_str() || sizeVal < 0) {
				m_errorCode = 400;
				m_state = STATE_ERROR;
				return true;
			}

			m_chunkSize = static_cast<size_t>(sizeVal);
			if (m_chunkSize == 0) {
				m_state = STATE_CHUNKED_TRAILER;
			} else {
				m_state = STATE_CHUNKED_DATA;
			}
		} else if (m_state == STATE_CHUNKED_DATA) {
			// Need m_chunkSize bytes + 2 bytes for \r\n
			if (m_rawBuffer.length() < m_chunkSize + 2)
				break;  // wait for entire chunk + CRLF

			m_body.append(m_rawBuffer, 0, m_chunkSize);
			m_rawBuffer.erase(0, m_chunkSize);

			if (m_rawBuffer.substr(0, 2) != "\r\n") {
				m_errorCode = 400;
				m_state = STATE_ERROR;
				return true;
			}
			m_rawBuffer.erase(0, 2);  // erase trailing CRLF

			if (maxBodySize > 0 && m_body.length() > maxBodySize) {
				m_errorCode = 413;
				m_state = STATE_ERROR;
				return true;
			}

			m_state = STATE_CHUNKED_SIZE;
		} else if (m_state == STATE_CHUNKED_TRAILER) {
			size_t crlf = m_rawBuffer.find("\r\n");
			if (crlf == std::string::npos) break;

			std::string line = m_rawBuffer.substr(0, crlf);
			m_rawBuffer.erase(0, crlf + 2);

			if (line.empty()) {
				m_state = STATE_COMPLETE;
				return true;
			}
			// Optional trailer headers can be parsed or ignored
		}
	}

	return (m_state == STATE_COMPLETE || m_state == STATE_ERROR);
}
