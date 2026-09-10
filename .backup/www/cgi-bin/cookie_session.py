#!/usr/bin/env python3
import os
import http.cookies

cookie_header = os.environ.get("HTTP_COOKIE", "")
cookies = http.cookies.SimpleCookie(cookie_header)

count = 1
if "cgi_visits" in cookies:
    try:
        count = int(cookies["cgi_visits"].value) + 1
    except ValueError:
        count = 1

print(f"Set-Cookie: cgi_visits={count}; Path=/; HttpOnly")
print("Content-Type: text/html\r\n")

print(f"""<!DOCTYPE html>
<html>
<head><title>CGI Cookie Session Demo</title></head>
<body style="font-family: sans-serif; padding: 40px; text-align: center;">
  <h2>🍪 CGI Cookie Session Demo</h2>
  <p>Raw <code>HTTP_COOKIE</code> received from Webserv: <b>{cookie_header or '(none)'}</b></p>
  <p style="font-size: 24px; color: #2563eb;">You have visited this CGI script <b>{count}</b> time(s)!</p>
  <p><a href="/cgi-bin/cookie_session.py">Refresh Page to Increment CGI Counter</a> | <a href="/">Return Home</a></p>
</body>
</html>""")

