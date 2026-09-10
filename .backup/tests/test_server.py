#!/usr/bin/env python3
import socket
import sys
import time
import urllib.request
import urllib.error

SERVER_HOST = "127.0.0.1"
PORT_8080 = 8080
PORT_8081 = 8081

passed = 0
failed = 0

def log_test(name, success, details=""):
    global passed, failed
    if success:
        passed += 1
        print(f"  [\033[32mPASS\033[0m] {name} {details}")
    else:
        failed += 1
        print(f"  [\033[31mFAIL\033[0m] {name} - {details}")

def send_raw_http(host, port, raw_request):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(10.0)
    s.connect((host, port))
    s.sendall(raw_request.encode('utf-8') if isinstance(raw_request, str) else raw_request)
    
    response = b""
    while True:
        try:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
        except socket.timeout:
            break
    s.close()
    return response.decode('utf-8', errors='replace')

def test_static_get():
    req = f"GET / HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Static GET / (200 OK)", "200 OK" in res and "42 Webserv" in res)

def test_autoindex():
    req = f"GET /uploads/ HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Autoindex /uploads/ (200 OK)", "200 OK" in res and "Index of /uploads/" in res)

def test_redirection():
    req = f"GET /redirect HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Redirection /redirect (301 Moved Permanently)", "301" in res and "Location: /" in res)

def test_404_error_page():
    req = f"GET /non_existent_page_12345 HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Custom 404 Error Page", "404 Not Found" in res and "Custom 404 Error Page" in res)

def test_method_not_allowed():
    req = f"DELETE / HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Method Not Allowed (405)", "405 Method Not Allowed" in res and "Allow:" in res)

def test_multi_port():
    req = f"GET / HTTP/1.1\r\nHost: second.local:{PORT_8081}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8081, req)
    log_test("Second Port 8081 (200 OK)", "200 OK" in res and "Second Server on Port 8081" in res)

def test_file_upload_and_delete():
    # 1. Upload file
    test_content = "Webserv automated test upload content."
    boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"
    body = (
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="file"; filename="pytest_upload.txt"\r\n'
        f"Content-Type: text/plain\r\n\r\n"
        f"{test_content}\r\n"
        f"--{boundary}--\r\n"
    )
    req = (
        f"POST /uploads HTTP/1.1\r\n"
        f"Host: localhost:{PORT_8080}\r\n"
        f"Content-Type: multipart/form-data; boundary={boundary}\r\n"
        f"Content-Length: {len(body)}\r\n"
        f"Connection: close\r\n\r\n"
        f"{body}"
    )
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("POST File Upload (201 Created)", "201 Created" in res)

    # 2. GET uploaded file
    req_get = f"GET /uploads/pytest_upload.txt HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res_get = send_raw_http(SERVER_HOST, PORT_8080, req_get)
    log_test("GET Uploaded File", "200 OK" in res_get and test_content in res_get)

    # 3. DELETE uploaded file
    req_del = f"DELETE /uploads/pytest_upload.txt HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res_del = send_raw_http(SERVER_HOST, PORT_8080, req_del)
    log_test("DELETE Uploaded File (204 No Content)", "204 No Content" in res_del)

    # 4. Verify file is gone
    res_gone = send_raw_http(SERVER_HOST, PORT_8080, req_get)
    log_test("Verify File Deleted (404 Not Found)", "404 Not Found" in res_gone)

def test_chunked_transfer():
    chunk1 = "Hello "
    chunk2 = "World from "
    chunk3 = "Chunked Transfer!"
    full_body = chunk1 + chunk2 + chunk3

    raw_req = (
        f"POST /uploads/chunked_test.txt HTTP/1.1\r\n"
        f"Host: localhost:{PORT_8080}\r\n"
        f"Transfer-Encoding: chunked\r\n"
        f"Connection: close\r\n\r\n"
        f"{hex(len(chunk1))[2:]}\r\n{chunk1}\r\n"
        f"{hex(len(chunk2))[2:]}\r\n{chunk2}\r\n"
        f"{hex(len(chunk3))[2:]}\r\n{chunk3}\r\n"
        f"0\r\n\r\n"
    )
    res = send_raw_http(SERVER_HOST, PORT_8080, raw_req)
    log_test("Transfer-Encoding: chunked unchunking", "201 Created" in res)

    # Clean up
    del_req = f"DELETE /uploads/chunked_test.txt HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    send_raw_http(SERVER_HOST, PORT_8080, del_req)

def test_cgi_get():
    req = f"GET /cgi-bin/hello.py?name=Antigravity&school=42 HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("CGI Python GET Execution", "200 OK" in res and "Python CGI Executed Successfully" in res and "name=Antigravity&school=42" in res)

def test_cgi_post():
    body = "message=Hello_from_automated_test"
    req = (
        f"POST /cgi-bin/hello.py HTTP/1.1\r\n"
        f"Host: localhost:{PORT_8080}\r\n"
        f"Content-Type: application/x-www-form-urlencoded\r\n"
        f"Content-Length: {len(body)}\r\n"
        f"Connection: close\r\n\r\n"
        f"{body}"
    )
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("CGI Python POST Execution", "200 OK" in res and body in res)

def test_cgi_bash():
    req = f"GET /cgi-bin/hello.sh?query=bash_test HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("CGI Bash Execution", "200 OK" in res and "Hello from Bash CGI!" in res)

def test_cgi_timeout():
    start = time.time()
    req = f"GET /cgi-bin/infinite.py HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    duration = time.time() - start
    log_test("CGI 5s Timeout (504 Gateway Timeout)", "504 Gateway Timeout" in res and 4.8 <= duration <= 8.0, f"(took {duration:.2f}s)")

def test_max_body_size():
    # Route /small has client_max_body_size 100 bytes
    oversized = "A" * 500
    req = (
        f"POST /small HTTP/1.1\r\n"
        f"Host: localhost:{PORT_8080}\r\n"
        f"Content-Length: {len(oversized)}\r\n"
        f"Connection: close\r\n\r\n"
        f"{oversized}"
    )
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("client_max_body_size Enforcement (413)", "413 Payload Too Large" in res)

def test_invalid_version():
    req = f"GET / HTTP/2.0\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("HTTP Version Not Supported (505)", "505 HTTP Version Not Supported" in res)

def test_malformed_request():
    req = "BAD_REQUEST_NO_VERSION\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Malformed Request (400 Bad Request)", "400 Bad Request" in res)

def test_keep_alive():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5.0)
    s.connect((SERVER_HOST, PORT_8080))

    req1 = f"GET / HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: keep-alive\r\n\r\n"
    s.sendall(req1.encode('utf-8'))
    res1 = s.recv(4096).decode('utf-8', errors='replace')

    req2 = f"GET /uploads/ HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    s.sendall(req2.encode('utf-8'))
    res2 = b""
    while True:
        chunk = s.recv(4096)
        if not chunk:
            break
        res2 += chunk
    s.close()
    res2 = res2.decode('utf-8', errors='replace')

    log_test("HTTP/1.1 Keep-Alive Connection", "200 OK" in res1 and "Index of /uploads/" in res2)

def test_missing_host_header():
    # RFC 7230 §5.4: HTTP/1.1 requests without Host header must return 400 Bad Request
    req = "GET / HTTP/1.1\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Missing Host Header in HTTP/1.1 (400 Bad Request)", "400 Bad Request" in res)

def test_extra_request_tokens():
    req = f"GET / HTTP/1.1 extra_token\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res = send_raw_http(SERVER_HOST, PORT_8080, req)
    log_test("Request Line Extra Tokens (400 Bad Request)", "400 Bad Request" in res)

def test_zombie_prevention():
    import subprocess
    output = subprocess.check_output(["ps", "-ef"]).decode('utf-8', errors='replace')
    defuncts = [line for line in output.splitlines() if "<defunct>" in line and "test_server" not in line]
    log_test("CGI Zombie Process Prevention", len(defuncts) == 0, f"(found {len(defuncts)} zombies)")

def test_cookies_and_sessions():
    # 1. Check inactive session initially
    res0 = send_raw_http(SERVER_HOST, PORT_8080, "GET /api/session HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n")
    check0 = "200 OK" in res0 and '"active":false' in res0

    # 2. Login to create session and get Set-Cookie
    login_body = "username=TestUser42"
    login_req = (
        f"POST /api/session/login HTTP/1.1\r\n"
        f"Host: localhost:{PORT_8080}\r\n"
        f"Content-Type: application/x-www-form-urlencoded\r\n"
        f"Content-Length: {len(login_body)}\r\n"
        f"Connection: close\r\n\r\n"
        f"{login_body}"
    )
    res_login = send_raw_http(SERVER_HOST, PORT_8080, login_req)
    check_login = "200 OK" in res_login and "Set-Cookie: session_id=" in res_login and "TestUser42" in res_login

    # Extract session_id
    cookie_val = ""
    for line in res_login.splitlines():
        if line.lower().startswith("set-cookie: session_id="):
            cookie_val = line.split(";")[0].split("=")[1].strip()
            break

    # 3. GET /api/session with Cookie header (visit 2)
    req_visit2 = f"GET /api/session HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nCookie: session_id={cookie_val}\r\nConnection: close\r\n\r\n"
    res_visit2 = send_raw_http(SERVER_HOST, PORT_8080, req_visit2)
    check_visit2 = "200 OK" in res_visit2 and '"active":true' in res_visit2 and '"visits":2' in res_visit2 and "TestUser42" in res_visit2

    # 4. GET /api/session again with Cookie header (visit 3)
    res_visit3 = send_raw_http(SERVER_HOST, PORT_8080, req_visit2)
    check_visit3 = "200 OK" in res_visit3 and '"visits":3' in res_visit3

    # 5. POST /api/session/logout to destroy session and clear cookie
    req_logout = f"POST /api/session/logout HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nCookie: session_id={cookie_val}\r\nConnection: close\r\n\r\n"
    res_logout = send_raw_http(SERVER_HOST, PORT_8080, req_logout)
    check_logout = "200 OK" in res_logout and "Max-Age=0" in res_logout

    # 6. GET /api/session after logout (must be inactive)
    res_after = send_raw_http(SERVER_HOST, PORT_8080, req_visit2)
    check_after = "200 OK" in res_after and '"active":false' in res_after

    all_passed = check0 and check_login and check_visit2 and check_visit3 and check_logout and check_after
    log_test("Cookies & Session Lifecycle (Login, Counter, Logout)", all_passed)

def test_cgi_cookie_session():
    # 1. First CGI visit without cookies
    req1 = f"GET /cgi-bin/cookie_session.py HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nConnection: close\r\n\r\n"
    res1 = send_raw_http(SERVER_HOST, PORT_8080, req1)
    check1 = "200 OK" in res1 and "Set-Cookie: cgi_visits=1" in res1 and "visited this CGI script <b>1</b> time(s)" in res1

    # 2. Second CGI visit with Cookie header
    req2 = f"GET /cgi-bin/cookie_session.py HTTP/1.1\r\nHost: localhost:{PORT_8080}\r\nCookie: cgi_visits=1\r\nConnection: close\r\n\r\n"
    res2 = send_raw_http(SERVER_HOST, PORT_8080, req2)
    check2 = "200 OK" in res2 and "Set-Cookie: cgi_visits=2" in res2 and "visited this CGI script <b>2</b> time(s)" in res2

    log_test("CGI Cookie Session Handling (HTTP_COOKIE & Set-Cookie)", check1 and check2)

def main():
    print("========================================")
    print("      Webserv Automated Test Suite      ")
    print("========================================")

    test_static_get()
    test_autoindex()
    test_redirection()
    test_404_error_page()
    test_method_not_allowed()
    test_multi_port()
    test_file_upload_and_delete()
    test_chunked_transfer()
    test_cgi_get()
    test_cgi_post()
    test_cgi_bash()
    test_cgi_timeout()
    test_zombie_prevention()
    test_max_body_size()
    test_invalid_version()
    test_malformed_request()
    test_missing_host_header()
    test_extra_request_tokens()
    test_keep_alive()
    test_cookies_and_sessions()
    test_cgi_cookie_session()

    print("========================================")
    print(f"Results: {passed} PASSED, {failed} FAILED")
    print("========================================")
    sys.exit(0 if failed == 0 else 1)

if __name__ == "__main__":
    main()
