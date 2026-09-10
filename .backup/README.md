_This project has been created as part of the 42 curriculum by sklaokli._

# Webserv

A lightweight, high-performance, non-blocking HTTP/1.1 web server implemented from scratch in **C++98**, inspired by the architecture and configuration semantics of **NGINX**.

---

## Description

Webserv is an event-driven HTTP/1.1 server designed to demonstrate a deep understanding of POSIX network programming, asynchronous I/O multiplexing, and the HTTP protocol (RFC 2616, RFC 7230, RFC 3875).

The server is built with a single event loop powered by `epoll` on Linux, ensuring non-blocking operations across all network sockets and CGI inter-process pipes. It handles concurrent client requests, static website delivery, file uploads, HTTP directory listings (`autoindex`), custom error pages, and dynamic script execution via the Common Gateway Interface (CGI) without relying on external libraries or multithreading.

### Key Architectural Highlights

- **Single Event Loop (`epoll`)**: All client sockets, listening sockets, and CGI pipes (stdin and stdout) are multiplexed in one unified non-blocking event loop.
- **Pure C++98**: Strictly complies with the C++98 standard (`-std=c++98 -Wall -Wextra -Werror -pedantic`) with zero external libraries (no Boost).
- **HTTP/1.1 Compliant**:
  - Methods: `GET`, `POST`, `DELETE`.
  - Transfer-Encoding: `chunked` (with streaming de-chunking).
  - Accurate HTTP status codes (`200`, `201`, `204`, `301`, `302`, `400`, `403`, `404`, `405`, `413`, `500`, `502`, `504`, `505`).
  - Persistent connections (`Connection: keep-alive` vs `close`).
- **NGINX-style Configuration**: Multi-port virtual hosts, custom error pages, file upload endpoints, URL redirections, autoindexing, and body size limits.
- **Bonus Features**:
  - **Multiple CGI Interpreters**: Fully dynamic multi-CGI dispatch by extension (supports Python `.py`, Bash `.sh`, PHP `php-cgi`).
  - **Cookies & Session Management**: In-memory `SessionManager` issuing `Set-Cookie` / parsing `Cookie`, session visit counter, login/logout, and CGI `HTTP_COOKIE` integration.
- **Robustness**: Tested with `siege` sustaining **10,000+ requests/second** at **100.00% availability** with **0 leaks** under Valgrind.

---

## Instructions

### Compilation

Build the executable using the provided `Makefile`:

```bash
make
```

Available Makefile rules:

- `make`: Compiles the binary `webserv`.
- `make clean`: Removes intermediate object files (`bin/`).
- `make fclean`: Removes object files and the `webserv` binary.
- `make re`: Performs a clean rebuild.

### Execution

Run the server with a configuration file:

```bash
./webserv [configuration_file]
```

If no configuration file is specified, `./webserv` defaults to `conf/default.conf`.

Example:

```bash
./webserv conf/default.conf
```

### Running the Test Suite

Webserv includes a full integration test suite covering static files, autoindex, upload, delete, chunked decoding, CGI execution, CGI timeout protection, error responses, and keep-alive:

```bash
# Terminal 1: Run server
./webserv conf/default.conf

# Terminal 2: Run automated tests
python3 tests/test_server.py
```

### Stress Testing

Verify performance and resilience using `siege`:

```bash
siege -b -c 25 -t 10s http://127.0.0.1:8080/
```

---

## Configuration File Syntax

The configuration format mirrors NGINX:

```nginx
server {
    listen 8080;
    host 127.0.0.1;
    server_name localhost webserv.local;
    client_max_body_size 10M;

    root ./www;
    index index.html;

    error_page 404 /errors/404.html;
    error_page 500 502 504 /errors/500.html;

    location / {
        allow_methods GET POST;
        autoindex on;
        index index.html;
    }

    location /uploads {
        allow_methods GET POST DELETE;
        root ./www/uploads;
        autoindex on;
        upload_enable on;
        upload_store ./www/uploads;
    }

    location /redirect {
        return 301 /;
    }

    location /cgi-bin {
        allow_methods GET POST;
        root ./www/cgi-bin;
        cgi_ext .py /usr/bin/python3;
        cgi_ext .sh /bin/bash;
    }
}
```

---

## Resources

### References & RFC Documentation

- [RFC 2616: Hypertext Transfer Protocol -- HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [RFC 7230: HTTP/1.1 Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 3875: The Common Gateway Interface (CGI) Version 1.1](https://datatracker.ietf.org/doc/html/rfc3875)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- Linux Manual Pages: `epoll(7)`, `epoll_create(2)`, `epoll_ctl(2)`, `epoll_wait(2)`, `fcntl(2)`, `socket(2)`, `fork(2)`, `dup2(2)`, `pipe(2)`.

### Use of Artificial Intelligence

Artificial Intelligence (Antigravity by Google DeepMind) was utilized during the development of this project:

- **Architecture Planning**: Designing the decoupled 5-tier architecture (Configuration, Poller Event Loop, HTTP Parser, Router, CGI Subprocess) and mapping requirements from `en.subject.pdf`.
- **Boilerplate and MIME Tables**: Generating MIME type maps and HTML error page templates.
- **Integration Test Suite**: Developing the comprehensive automated Python test harness (`tests/test_server.py`) to systematically validate RFC compliance, edge cases (chunked transfer decoding, CGI timeouts, client_max_body_size enforcement), and socket cleanups.
- **Refinement**: Reviewing non-blocking edge cases (such as draining socket buffers on 413 rejections and managing child processes with `WNOHANG`).
