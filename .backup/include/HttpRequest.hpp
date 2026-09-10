#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <map>
#include <string>
#include <vector>

enum RequestParseState {
	STATE_REQUEST_LINE,
	STATE_HEADERS,
	STATE_BODY,
	STATE_CHUNKED_SIZE,
	STATE_CHUNKED_DATA,
	STATE_CHUNKED_TRAILER,
	STATE_COMPLETE,
	STATE_ERROR
};

class HttpRequest {
public:
	HttpRequest();
	~HttpRequest();

	void reset();
	// Feeds raw incoming data into the parser.
	// Returns true if parsing is complete or in error state.
	bool parseChunk(const std::string& data, size_t maxBodySize = 0);

	const std::string& getMethod() const;
	const std::string& getUri() const;
	const std::string& getPath() const;
	const std::string& getQueryString() const;
	const std::string& getVersion() const;
	const std::string& getBody() const;
	const std::map<std::string, std::string>& getHeaders() const;

	bool hasHeader(const std::string& name) const;
	std::string getHeader(const std::string& name) const;

	bool isComplete() const;
	bool hasError() const;
	int getErrorCode() const;

	bool isKeepAlive() const;

	bool hasCookie(const std::string& name) const;
	std::string getCookie(const std::string& name) const;
	const std::map<std::string, std::string>& getCookies() const;

private:
	RequestParseState m_state;
	int m_errorCode;

	std::string m_rawBuffer;
	std::string m_method;
	std::string m_uri;
	std::string m_path;
	std::string m_queryString;
	std::string m_version;
	std::map<std::string, std::string> m_headers;
	std::map<std::string, std::string> m_cookies;
	std::string m_body;

	size_t m_contentLength;
	bool m_isChunked;
	size_t m_chunkSize;

	bool parseRequestLine(const std::string& line);
	bool parseHeaderLine(const std::string& line);
	void parseUri();
	void parseCookies(const std::string& cookieHeader);
};

#endif
