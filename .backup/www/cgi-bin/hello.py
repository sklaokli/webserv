#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r\n\r\n")

method = os.environ.get("REQUEST_METHOD", "GET")
query = os.environ.get("QUERY_STRING", "")

post_body = ""
if method == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        post_body = sys.stdin.read(content_length)

print("<!DOCTYPE html>")
print("<html><head><title>Python CGI Response</title></head>")
print("<body style='font-family: monospace; padding: 30px;'>")
print("<h1>Python CGI Executed Successfully!</h1>")
print(f"<p><strong>Method:</strong> {method}</p>")
print(f"<p><strong>Query String:</strong> {query}</p>")

if method == "POST":
    print(f"<p><strong>POST Body:</strong> {post_body}</p>")

print("<h2>Environment Variables:</h2><ul>")
for k, v in sorted(os.environ.items()):
    if k.startswith("HTTP_") or k in ("REQUEST_METHOD", "QUERY_STRING", "SCRIPT_NAME", "PATH_INFO", "REMOTE_ADDR"):
        print(f"<li><strong>{k}</strong> = {v}</li>")
print("</ul>")
print("<hr><a href='/'>Back to Home</a>")
print("</body></html>")

