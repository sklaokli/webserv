#!/usr/bin/env python3
"""Component acceptance tests: real Config parser + C++ Router + HTTP serialization."""
import base64
import html.parser
import os
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest

DRIVER = str(Path(sys.argv.pop(1)).resolve())

class Links(html.parser.HTMLParser):
    def __init__(self, text):
        super().__init__(convert_charrefs=True)
        self.links = []
        self.feed(text)
    def handle_starttag(self, tag, attrs):
        if tag == "a":
            self.links.extend(value for key, value in attrs if key == "href")

class RouterTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="webserv-router-")
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.root = self.base / "site"
        self.alias = self.base / "alias"
        self.root.mkdir()
        self.alias.mkdir()
        for folder in ["assets", "listing", "listing/sub", "off", "indexed",
                       "restricted", "private", "cgidir", "bad-index"]:
            (self.root / folder).mkdir()
        (self.root / "index.html").write_text("<h1>home</h1>")
        (self.root / "assets/theme.css").write_text("body { color: red; }")
        (self.root / "assets/app.js").write_text("let hello = true;")
        (self.root / "assets/unknown.blob").write_bytes(b"\x00\xff\x01")
        self.png = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAusB9Y9ZQmcAAAAASUVORK5CYII=")
        (self.root / "assets/image.PNG").write_bytes(self.png)
        (self.root / "assets/image.jpg").write_bytes(b"\xff\xd8\xff\xd9")
        (self.root / "assets/file.pdf").write_bytes(b"%PDF-1.4\n\x00")
        (self.root / "custom404.html").write_text("<h1>custom missing</h1>")
        (self.root / "custom405.html").write_text("<h1>custom method</h1>")
        (self.root / "indexed/home.html").write_text("configured index")
        (self.root / "indexed/index.html").write_text("wrong index")
        (self.root / "cgidir/index.py").write_text("CGI SOURCE SECRET")
        (self.root / "script.py").write_text("CGI SOURCE SECRET")
        (self.alias / "file.txt").write_text("alias content")
        (self.alias / "index.html").write_text("alias index")
        self.special = 'a <&"?#%.txt'
        (self.root / "listing" / self.special).write_text("special file")
        (self.root / "listing/large.txt").write_bytes(b"x" * 2048)
        (self.root / "listing/sub/index.html").write_text("subdirectory index")
        self.secret = self.base / "secret.txt"
        self.secret.write_text("OUTSIDE SECRET")
        (self.root / "leak").symlink_to(self.secret)
        (self.root / "escape").symlink_to(self.base, target_is_directory=True)
        (self.root / "listing/link").symlink_to(self.secret)
        (self.root / "internal-link").symlink_to(self.root / "index.html")
        (self.root / "broken").symlink_to(self.base / "not-there")
        os.mkfifo(self.root / "pipe")
        (self.root / "denied.txt").write_text("hidden")
        (self.root / "denied.txt").chmod(0)
        (self.root / "private/secret.txt").write_text("private")
        (self.root / "private").chmod(0o600)
        self.addCleanup((self.root / "private").chmod, 0o700)
        self.addCleanup((self.root / "denied.txt").chmod, 0o600)
        (self.root / "bad-index/index.html").symlink_to(self.secret)
        self.conf = self.base / "test.conf"
        self.conf.write_text(f"""
server {{
    listen 18999;
    root {self.root};
    index index.html;
    error_page 404 /custom404.html;
    error_page 405 /custom405.html;
    location / {{ allow_methods GET HEAD; autoindex off; cgi_ext .py /usr/bin/python3; }}
    location /listing/ {{ allow_methods GET HEAD; autoindex on; }}
    location /off/ {{ autoindex off; }}
    location /indexed/ {{ index home.html; }}
    location /restricted/ {{ allow_methods POST DELETE; }}
    location /move {{ return 301 /new-path; }}
    location /assets/ {{ root {self.root}; allow_methods GET HEAD; }}
    location /alias/ {{ alias {self.alias}; }}
    location /alias/deep/ {{ return 302 /deep-wins; }}
    location /prefix {{ return 301 /literal-prefix; }}
    location /cgidir/ {{ index index.py; cgi_ext .py /usr/bin/python3; }}
    location /bad-index/ {{ autoindex on; }}
}}
""")

    def run_raw(self, method, target, conf=None, repeat=None, preexec_fn=None):
        args = [DRIVER, str(conf or self.conf), method, target]
        if repeat is not None:
            args.append(str(repeat))
        return subprocess.run(args, capture_output=True, timeout=5,
                              preexec_fn=preexec_fn)

    def request(self, target, method="GET", conf=None, repeat=None, preexec_fn=None):
        result = self.run_raw(method, target, conf, repeat, preexec_fn)
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        head, body = result.stdout.split(b"\r\n\r\n", 1)
        lines = head.decode("utf-8").split("\r\n")
        status = int(lines[0].split(" ")[1])
        headers = dict(line.split(": ", 1) for line in lines[1:])
        if method != "HEAD" and status != 304:
            self.assertEqual(int(headers["Content-Length"]), len(body))
        return status, headers, body

    def test_home_and_query(self):
        for url in ["/", "/?q=ignored", "//./", "/index.html?q=/etc/passwd"]:
            with self.subTest(url=url):
                status, headers, body = self.request(url)
                self.assertEqual(headers["Content-Type"], "text/html")
                self.assertEqual((status, body), (200, b"<h1>home</h1>"))

    def test_longest_literal_prefix(self):
        self.assertEqual(self.request("/alias/deep/x")[1]["Location"], "/deep-wins")
        self.assertEqual(self.request("/prefix-suffix")[1]["Location"], "/literal-prefix")

    def test_root_appends_full_uri(self):
        self.assertEqual(self.request("/assets/theme.css")[2], b"body { color: red; }")

    def test_alias_replaces_prefix(self):
        self.assertEqual(self.request("/alias/file.txt")[2], b"alias content")
        self.assertEqual(self.request("/alias/")[2], b"alias index")

    def test_redirect(self):
        status, headers, body = self.request("/move/anything")
        self.assertEqual((status, headers["Location"], body), (301, "/new-path", b""))

    def test_405_and_allow(self):
        status, headers, body = self.request("/restricted/file", "GET")
        self.assertEqual(status, 405)
        self.assertEqual(headers["Allow"], "POST, DELETE")
        self.assertIn(b"custom method", body)
        self.assertEqual(self.request("/move", "DELETE")[0], 405)
        self.assertEqual(self.request("/off/", "HEAD")[0], 405)

    def test_allowed_non_static_methods_delegate(self):
        for method in ["POST", "DELETE"]:
            result = self.run_raw(method, "/restricted/new")
            self.assertEqual(result.returncode, 0)
            self.assertTrue(result.stdout.startswith(b"DELEGATE METHOD\n"))

    def test_cgi_never_leaks_source(self):
        for url in ["/script.py", "/cgidir/"]:
            result = self.run_raw("GET", url)
            self.assertTrue(result.stdout.startswith(b"DELEGATE CGI\n"))
            self.assertNotIn(b"CGI SOURCE SECRET", result.stdout)

    def test_traversal(self):
        for url in ["/../../../../etc/passwd", "/a/../index.html",
                    "/%2e%2e/secret.txt", "/%2E%2E%2Fsecret.txt",
                    "/alias/%2e%2e/secret.txt", "/listing/.%2e/secret.txt",
                    "/../move"]:
            with self.subTest(url=url):
                status, _, body = self.request(url)
                self.assertIn(status, [403, 404])
                self.assertNotIn(b"OUTSIDE SECRET", body)

    def test_malformed_targets(self):
        for url in ["/bad%", "/bad%zz", "/%00", "/%0d%0aInjected:yes",
                    "/back\\slash", "/%5c", "relative", "/a#fragment",
                    "/?a=\r\nInjected:yes"]:
            with self.subTest(url=url):
                self.assertEqual(self.request(url)[0], 400)

    def test_single_decode_and_normalization(self):
        self.assertEqual(self.request("/%2569ndex.html")[0], 404)
        self.assertEqual(self.request("/%69ndex.html")[0], 200)
        self.assertEqual(self.request("/assets//./theme.css")[0], 200)
        self.assertEqual(self.request("/listing/.")[0], 200)

    def test_mime_and_binary(self):
        cases = {
            "theme.css": ("text/css", b"body { color: red; }"),
            "app.js": ("text/javascript", b"let hello = true;"),
            "image.PNG": ("image/png", self.png),
            "image.jpg": ("image/jpeg", b"\xff\xd8\xff\xd9"),
            "file.pdf": ("application/pdf", b"%PDF-1.4\n\x00"),
            "unknown.blob": ("application/octet-stream", b"\x00\xff\x01"),
        }
        for filename, (mime, content) in cases.items():
            with self.subTest(filename=filename):
                status, headers, body = self.request("/assets/" + filename)
                self.assertEqual((status, headers["Content-Type"], body),
                                 (200, mime, content))

    def test_head(self):
        _, get_headers, get_body = self.request("/assets/image.PNG")
        status, headers, body = self.request("/assets/image.PNG", "HEAD")
        self.assertEqual((status, body), (200, b""))
        self.assertEqual(headers["Content-Length"], str(len(get_body)))
        self.assertEqual(headers, get_headers)
        self.assertEqual(self.request("/not-found", "HEAD")[2], b"")

    def test_directory_slash_redirect(self):
        status, headers, _ = self.request("/listing?q=a%20b")
        self.assertEqual((status, headers["Location"]), (301, "/listing/?q=a%20b"))
        self.assertEqual(self.request("/index.html/")[0], 404)

    def test_configured_index(self):
        self.assertEqual(self.request("/indexed/")[2], b"configured index")

    def test_autoindex_off(self):
        self.assertEqual(self.request("/off/")[0], 403)

    def test_autoindex_markup_and_links(self):
        status, headers, body = self.request("/listing/")
        self.assertEqual(status, 200)
        self.assertEqual(headers["Content-Type"], "text/html; charset=utf-8")
        text = body.decode()
        self.assertTrue(text.startswith("<!DOCTYPE html>"))
        self.assertIn("2.0 KiB", text)
        self.assertRegex(text, r"\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} UTC")
        self.assertIn("&lt;&amp;&quot;", text)
        self.assertNotIn("OUTSIDE SECRET", text)
        links = Links(text).links
        self.assertNotIn("/listing/link", links)
        special_link = next(link for link in links if "%3C" in link)
        self.assertIn("%25", special_link)
        self.assertEqual(self.request(special_link)[2], b"special file")
        for link in links:
            self.assertEqual(self.request(link)[0], 200, link)

    def test_missing_and_custom_error(self):
        status, _, body = self.request("/missing")
        self.assertEqual(status, 404)
        self.assertIn(b"custom missing", body)
        self.assertNotIn(str(self.root).encode(), body)

    def test_custom_error_failure_falls_back(self):
        (self.root / "custom404.html").unlink()
        status, _, body = self.request("/missing")
        self.assertEqual(status, 404)
        self.assertIn(b"Not Found", body)

    def test_custom_error_cannot_escape(self):
        (self.root / "custom404.html").unlink()
        (self.root / "custom404.html").symlink_to(self.secret)
        status, _, body = self.request("/missing")
        self.assertEqual(status, 404)
        self.assertNotIn(b"OUTSIDE SECRET", body)

    def test_symlinks_and_special_files(self):
        for url in ["/leak", "/escape/secret.txt", "/broken",
                    "/internal-link", "/pipe", "/bad-index/"]:
            with self.subTest(url=url):
                status, _, body = self.request(url)
                self.assertIn(status, [403, 404])
                self.assertNotIn(b"OUTSIDE SECRET", body)

    def test_permissions(self):
        for url in ["/denied.txt", "/private/secret.txt"]:
            self.assertEqual(self.request(url)[0], 403)

    def test_root_without_location(self):
        config = self.base / "fallback.conf"
        config.write_text(f"server {{ listen 18000; root {self.root}; }}")
        self.assertEqual(self.request("/", conf=config)[2], b"<h1>home</h1>")
        self.assertEqual(self.request("/", "DELETE", conf=config)[1]["Allow"], "GET")

    def test_alias_parser_validation(self):
        for directives in [f"root {self.root}; alias {self.alias};",
                           f"alias {self.alias}; root {self.root};",
                           f"alias {self.alias}; alias {self.alias};"]:
            config = self.base / "invalid.conf"
            config.write_text("server { listen 18000; location / { " + directives + " } }")
            self.assertEqual(self.run_raw("GET", "/", config).returncode, 2)

    def test_descriptors_are_closed(self):
        def low_limit():
            resource.setrlimit(resource.RLIMIT_NOFILE, (48, 48))
        for url in ["/index.html", "/listing/", "/missing", "/leak"]:
            status, _, _ = self.request(url, repeat=300, preexec_fn=low_limit)
            self.assertIn(status, [200, 403, 404])

if __name__ == "__main__":
    unittest.main(verbosity=2)
