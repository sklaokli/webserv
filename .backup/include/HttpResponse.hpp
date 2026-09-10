#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <map>
#include <string>
#include <vector>

class HttpResponse {
public:
	HttpResponse();
	~HttpResponse();

	void setStatus(int statusCode, const std::string& statusMessage = "");
	int getStatusCode() const;
	const std::string& getStatusMessage() const;

	void setHeader(const std::string& key, const std::string& value);
	bool hasHeader(const std::string& key) const;
	std::string getHeader(const std::string& key) const;

	void setCookie(const std::string& name, const std::string& value,
	    const std::string& path = "/", int maxAge = -1, bool httpOnly = true);

	void setBody(const std::string& body);
	const std::string& getBody() const;

	void setRedirect(int statusCode, const std::string& location);

	std::string serialize() const;

	static std::string getDefaultStatusMessage(int statusCode);
	static std::string generateDefaultErrorPage(
	    int statusCode, const std::string& customMsg = "");

private:
	int m_statusCode;
	std::string m_statusMessage;
	std::map<std::string, std::string> m_headers;
	std::vector<std::string> m_cookies;
	std::string m_body;
};

#endif
