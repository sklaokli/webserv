# Task 01: URL Router, Static Files และ AutoIndex

## สถานะงานและตำแหน่งไฟล์

ทำใน `B:\webserv-wsl\webserv` ซึ่ง WSL เข้าถึงได้ที่
`/mnt/webserv-wsl/webserv` บน branch `feat/router-static-autoindex`

โค้ดก่อนเริ่มงานมี config parser และ Logger แล้ว แต่ `src/main.cpp` ยังไม่มี
HTTP request parser, socket accept loop หรือ `Server::run()`
งานนี้เพิ่มโมดูล routing ที่นำไปใช้กับ server ได้ และทดสอบด้วย parser/config จริง
ผ่าน executable `bin/router_demo` ซึ่งรับ method กับ request target จาก command line

**`./webserv` ยังไม่เปิด port และยังเปิดเว็บนี้ผ่าน browser ไม่ได้**
การทำงานที่พิสูจน์แล้วเป็นระดับ component: parse config → route → เปิดไฟล์/สร้าง listing →
serialize HTTP response ทั้ง header และ binary body
ส่วนการรับ request จาก socket, poll, keep-alive, timeout, CGI และ upload/delete ต้องเชื่อมจากงานของทีม

## อ่านโจทย์อะไร และมีผลต่อการออกแบบอย่างไร

อ่าน `en.subject.pdf` เวอร์ชัน 24.1 ครบ 15 หน้า โดยส่วนที่เกี่ยวข้องคือ:

| หน้า PDF | ข้อกำหนด | สิ่งที่ทำใน task นี้ |
|---|---|---|
| 4 | C++98, -Wall -Wextra -Werror, ห้าม external libraries | ใช้ STL กับ POSIX/Linux functions ที่โจทย์อนุญาต |
| 7 | รายการ external functions | ใช้ open, close, read, stat, access, opendir, readdir, closedir; ไม่ใช้ realpath, lstat, readlink, openat หรือ std::filesystem |
| 9 | static website, default errors, disk files ยกเว้น poll | อ่านเฉพาะ regular file หลังตรวจชนิด; ไม่อ่าน FIFO/socket เป็น static file |
| 11 | allowed methods, redirects, location mapping, directory listing และ index | Router, config alias, index fallback, AutoIndex |
| 12 | ต้องมี config และไฟล์สาธิต | conf/routing-demo.conf และ tests/fixtures |
| 13 | README ต้องเป็นภาษาอังกฤษและอธิบายการใช้ AI | เพิ่มคำแนะนำ task และลิงก์คู่มือนี้ใน README; README ของโปรเจกต์เต็มยังต้องเติมรายชื่อทีมและส่วนอื่นตอนรวมงาน |

โจทย์ยกตัวอย่าง `/kapouet/pouic` ที่ map ไป `/tmp/www/pouic`
ในงานนี้เขียนได้ด้วย `location /kapouet/ { alias /tmp/www; }`
ส่วน directive `root` ใช้ความหมายแบบ NGINX คือเอา URL เต็มต่อท้าย root

## ภาพรวม request หนึ่งครั้ง

```text
method + raw request target
        |
decode URL ครั้งเดียว + ตรวจรูปแบบ + จัด path ให้แน่นอน
        |
เลือก location ที่ prefix ยาวที่สุด
        |
ตรวจ allow_methods -> 405 พร้อม Allow หากไม่อนุญาต
        |
ตรวจ return -> response redirect
        |
resolve root/alias
        |
POST/DELETE/PUT -> METHOD_HANDLER ให้ทีมจัดการ method ต่อ
configured CGI -> CGI_HANDLER เพื่อไม่เสิร์ฟ source code
        |
เปิด resource ภายใน base ที่อนุญาต
        |
ไฟล์ -> อ่านแบบ binary และกำหนด MIME
directory -> เติม slash -> ลอง index -> AutoIndex หรือ 403
```

หาก path ไม่ปลอดภัย จะหยุดก่อนตรวจ method และ redirect
จึงไม่สามารถใช้ redirect route กลบ `/../` ได้

## 1. เลือก location ด้วย longest prefix

`ServerConfig::findLocation()` เปรียบเทียบ path ที่ normalize แล้วกับทุก location
และเลือก prefix ที่ยาวที่สุด โดยไม่ขึ้นกับลำดับใน config

ตัวอย่างมี `/`, `/assets/`, `/assets/private/`:
request `/assets/private/a.txt` จะเลือก `/assets/private/`

เป็น **literal prefix**: `location /img` จับ `/images` ด้วย
ถ้าต้องการขอบเขต directory ให้ใช้ `location /img/`
constructor ของ LocationConfig จึงเก็บ trailing slash ไว้ ไม่ตัดทิ้งเหมือนเดิม

`findLocation()` รับ path ที่ normalize แล้ว ไม่รับ raw query string
ให้เรียกผ่าน `Router::route()` เพื่อทำ validation ก่อน
ถ้าไม่มี location จับได้ จะใช้ root/index ของ server, อนุญาต GET และปิด AutoIndex

## 2. ความต่างระหว่าง root กับ alias

| Config | Request | ไฟล์ที่ต้องหา |
|---|---|---|
| root /srv/www; | /assets/app.css | /srv/www/assets/app.css |
| location /assets/ { root /srv/www; } | /assets/app.css | /srv/www/assets/app.css |
| location /assets/ { alias /srv/files; } | /assets/app.css | /srv/files/app.css |

`root` ต่อ path เต็ม ส่วน `alias` ตัด location prefix ก่อนต่อ suffix
งานนี้ alias เป็น **directory mapping** ไม่ใช่ alias ไปไฟล์เดี่ยว
alias ใช้ได้ใน location เท่านั้น และห้ามเขียน root กับ alias ใน location เดียวกัน
parser ตรวจทั้งสองลำดับและ duplicate alias แล้ว

Location ที่ไม่มี root ยังคง inherit root จาก server ตามระบบเดิม
เมื่อมี alias Router จะเลือก alias เป็น base แทน inherited root
path ใน config ที่เป็น relative จะอ้างอิง current working directory ของ process
จึงควรรันคำสั่งจาก repository root

## 3. Method และ redirect

ชื่อ directive ของ parser นี้คือ `allow_methods`
ส่วนคำว่า `allowed_methods` ใน task หมายถึงรายการ method ที่อนุญาต

```nginx
location /write-only/ {
    allow_methods POST DELETE;
}
location /old {
    return 301 /new-path;
}
```

GET /write-only/file ให้ status 405 และ `Allow: POST, DELETE`
ถ้ามี custom error สำหรับ 405 จะใช้ body นั้น แต่ยังเก็บ status และ Allow ไว้

GET /old ให้ 301 และ `Location: /new-path`
ตรวจ method ก่อน redirect ดังนั้น DELETE /old ที่ไม่ได้อนุญาตจะได้ 405
redirect URL ที่มี control characters จะได้ 500 เพื่อกันการแทรก response headers

HEAD ต้องประกาศอนุญาตเองเช่น `allow_methods GET HEAD;`
response ของ HEAD ใช้ Content-Length เท่ากับ GET แต่ serializer ไม่ส่ง body

POST/DELETE/PUT ที่อนุญาตจะได้ `METHOD_HANDLER`
ไม่ได้อ้างว่าทำ upload/delete สำเร็จ และไม่ส่ง 405 ให้ method ที่ config อนุญาตแล้ว

## 4. ป้องกัน directory traversal

### URL normalization

`PathResolver::normalizeTarget()`:

1. รับ origin-form target ที่ขึ้นต้นด้วย / เช่น /img/a.png?q=1
2. แยก query ออกจาก filesystem path; query ไม่ใช้หาไฟล์
3. decode %HH เพียงครั้งเดียว
4. ปฏิเสธ malformed percent, NUL, control characters, backslash และ raw fragment ด้วย 400
5. ปฏิเสธทุก component ที่เป็น .. ด้วย 403 รวมถึงแบบ encoded
6. ยุบ // และ /./ และเก็บ slash ท้ายที่สื่อว่าเป็น directory

ตัวอย่าง:

| Request | ผล |
|---|---|
| /assets//./app.css | canonical URL เป็น /assets/app.css |
| /%69ndex.html | /index.html |
| /../../etc/passwd | 403 |
| /%2e%2e/etc/passwd | 403 |
| /safe/../index.html | 403 ตามนโยบายปฏิเสธ .. ทั้งหมด |
| /%00 | 400 |
| /%2569ndex.html | หาไฟล์ชื่อ %69ndex.html จริง ไม่ decode เป็น index.html รอบสอง |

### Filesystem containment และ symlink

แค่ลบ ../ ไม่พอ: symlink ใต้ www อาจชี้ออกไป /etc ได้
จึงใช้แนวทาง **ไม่ติดตาม symlink ใต้ configured base**

`openBeneath()` เปิด base directory ก่อน แล้วถือ file descriptor ไว้
แต่ละ component ถูกเปิดผ่าน `/proc/self/fd/<parent-fd>/<name>` ด้วย O_NOFOLLOW
เปิด component ถัดไปสำเร็จจึงปิด descriptor ของ parent เดิม
วิธีนี้ยึด directory ที่เปิดแล้ว และไม่เชื่อชื่อ parent เดิมซ้ำระหว่างตรวจและเปิด

ใช้ `stat("/proc/self/fd/<fd>")` ตรวจ object ที่เปิดจริง
แทนการ stat pathname ก่อนแล้วเปิด pathname เดิมอีกครั้ง
`FileHandle` ปิด descriptor อัตโนมัติด้วย destructor แม้ return ก่อนจบหรือเกิด exception

**ข้อจำกัด platform:** การเปิดแบบนี้รองรับ Linux/WSL2 ที่มี procfs
ยังไม่ได้รองรับ macOS และจะ fail closed หาก /proc/self/fd ใช้ไม่ได้
ต้องทดสอบใหม่บนเครื่องประเมิน หากต้องใช้ macOS ต้องตกลงวิธี secure filesystem access เพิ่มเติม
configured base เป็นค่าที่ผู้ดูแลเชื่อถือได้ อนุญาตให้ base เองชี้ผ่าน symlink ได้;
ทุก descendant จาก request ถูกห้าม symlink รวมถึง symlink ที่ชี้กลับเข้าภายใน root
hard link/bind mount ที่ผู้ดูแลวางไว้ภายใน document root ถือเป็นเนื้อหาของ root
ควรจัด root ให้มีเฉพาะไฟล์ที่ตั้งใจเผยแพร่

## 5. ตรวจชนิดไฟล์และ permission

- ENOENT/ENOTDIR จาก open/stat -> 404
- EACCES/EPERM/ELOOP -> 403
- pathname ยาวเกิน filesystem -> 414
- error อื่น -> 500
- ตรวจ read permission bits และสิทธิ์ traverse directory
- รับเฉพาะ regular file หรือ directory
- เปิด O_NONBLOCK เพื่อไม่ค้างเมื่อเจอ FIFO แล้วปฏิเสธ special files ก่อน read
- หลัง read ล้มเหลวคืน 500 โดยไม่ใช้ errno ตัดสิน ตามข้อกำหนด PDF
- ข้อผิดพลาดตอบข้อความทั่วไป ไม่แนบ filesystem path หรือ strerror ให้ client

Permission tests สร้าง fixture ใน /tmp ฝั่ง Linux เพื่อทดสอบ chmod จริง
การทำ chmod บน Windows drive อาจมีพฤติกรรมตาม mount options ของ drvfs ต่างจาก ext4

## 6. Static file, MIME และ directory index

`readRegular()` ใช้ read แล้ว append ตามจำนวน bytes ที่อ่านจริง
จึงไม่ตัด body เมื่อเจอ NUL และไม่แปลง line endings ของรูปภาพ/PDF

`MimeTypes::lookup()` ใช้นามสกุลแบบ case-insensitive:
html, css, js/mjs, txt, csv, json, xml, pdf, png, jpg/jpeg, gif, svg, ico,
webp, avif, woff/woff2, mp3, mp4, wasm และ zip
นามสกุลที่ไม่รู้จักใช้ application/octet-stream
response เพิ่ม X-Content-Type-Options: nosniff

กรณี directory:

1. URL ไม่ลงท้าย / -> 301 ไป URL ที่มี / และเก็บ query เดิม
2. ลอง index ที่กำหนดใน location หรือ inherit จาก server; ค่าเริ่มต้น index.html
3. ถ้า index ไม่มีจริง และ autoindex on -> สร้าง listing
4. ถ้า autoindex off -> 403
5. index อ่านไม่ได้/เป็น symlink/เป็น directory -> 403 ไม่ข้ามไปแสดง listing

index รองรับชื่อไฟล์เดี่ยวหนึ่งชื่อ เช่น home.html ตาม interface parser ปัจจุบัน
ชื่อ ../secret หรือ path หลายชั้นถูกปฏิเสธ
CGI extension ที่ config ระบุ ทั้ง direct request และ directory index จะส่งต่อ CGI_HANDLER
เพื่อไม่ส่ง source code กลับเป็น static content

## 7. Custom error page

`Router::error()` หา URI จาก server.getErrorPage(status)
แล้วโหลดใต้ **server root** โดยใช้ข้อกำหนดความปลอดภัยเดียวกับ static files
ไม่ได้ route error page ซ้ำ จึงไม่เกิด redirect หรือ error recursion
ไม่ใช้ alias ในการหา error page

ถ้า error page หาย, อ่านไม่ได้ หรือชี้ออกนอก root จะใช้ HTML เริ่มต้น
สถานะเดิมยังคงอยู่ เช่น custom404.html โหลดไม่ได้ ก็ยังตอบ 404
body เริ่มต้นไม่ใส่ raw URL หรือ filesystem path

## 8. AutoIndex

ใช้ opendir/readdir อ่าน directory ที่ถูกเปิดอย่างปลอดภัยแล้ว
ไม่แสดง . และ .. เป็น filesystem entries; สร้าง parent link จาก URL แทน
symlink, broken link, special file และ entry ที่เข้าไม่ได้จะถูกละไว้

เรียง directory ก่อน file แล้วเรียงชื่อตาม byte order
แต่ละแถวมีชื่อ/ลิงก์, modification time แบบ UTC และขนาด B/KiB/MiB/GiB/TiB
directory ใช้ขนาด - และมี / ต่อท้ายชื่อ

มี encoding สองแบบซึ่งมีหน้าที่ต่างกัน:

- ข้อความชื่อใน HTML: <, >, &, quote -> HTML entities เพื่อกันการแทรก markup
- href: space, #, ?, %, quote และอักขระอื่น -> percent encoding
- path ที่เป็น URL ใน link ใช้ normalized URL ไม่ใช่ filesystem path

เช่นชื่อ `hello world.txt` แสดงตามเดิม แต่ href เป็น `hello%20world.txt`
tests ใช้ HTMLParser ดึง href ออกมา แล้วส่งทุกลิงก์กลับเข้า C++ Router จริง

## 9. วิธี compile และทดสอบ

รันใน Ubuntu/WSL:

```bash
cd /mnt/webserv-wsl/webserv
make
make test-router
make route-demo
./bin/router_demo conf/routing-demo.conf GET /
./bin/router_demo conf/routing-demo.conf GET /assets/style.css
./bin/router_demo conf/routing-demo.conf GET /listing/
./bin/router_demo conf/routing-demo.conf GET /downloads/example.txt
./bin/router_demo conf/routing-demo.conf GET /old
./bin/router_demo conf/routing-demo.conf DELETE /
./bin/router_demo conf/routing-demo.conf GET /../../../../etc/passwd
./bin/router_demo conf/routing-demo.conf HEAD /index.html
```

router_demo พิมพ์ HTTP response ไป stdout; สำหรับ method/CGI ที่ต้องส่งต่อจะพิมพ์
DELEGATE METHOD หรือ DELEGATE CGI ซึ่งเป็นผลเฉพาะ test harness ไม่ใช่ HTTP response

เมื่อทีมเชื่อม network server แล้วจึงใช้:

```bash
curl --path-as-is -i http://127.0.0.1:8080/../../../../etc/passwd
curl -i -X DELETE http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/listing/
```

`--path-as-is` สำคัญเพราะ curl อาจยุบ ../ เองก่อนส่ง ทำให้ไม่ได้ทดสอบ server
คำสั่ง curl ด้านบนเป็นขั้นตอน integration ในอนาคต ยังไม่ใช่หลักฐานที่รันผ่านแล้ว

ผลที่รันจริงวันที่ 2026-09-23:

- make: ผ่าน -Wall -Wextra -Werror -std=c++98 -pedantic
- make test-router: 25 test methods ผ่าน มี subcases เพิ่มเติมด้าน malformed URL/MIME/traversal
- 300 requests ซ้ำต่อกรณี ภายใต้ RLIMIT_NOFILE=48 ผ่านสำหรับ file/listing/missing/symlink
- make ซ้ำ: Nothing to be done for 'all' จึงไม่มีการ relink โดยไม่จำเป็น
- git diff --check: ผ่าน

## 10. วิธีนำไปต่อกับงานทีม

```cpp
RouteResult result = Router::route(requestMethod, rawRequestTarget, serverConfig);

if (result.action == RouteResult::RESPONSE) {
    std::string bytes = result.response.serialize(requestMethod == "HEAD");
    // เก็บ bytes ลง output buffer ของ connection
    // ให้ event loop ส่งด้วย send() หลัง poll แจ้ง writable
} else if (result.action == RouteResult::CGI_HANDLER) {
    // ส่งต่อ CGI layer; ตรวจไฟล์และ process policy ของ CGI แยกต่างหาก
} else {
    // ส่งต่อ POST/DELETE/PUT handler ตาม method
}
```

location pointer ใน result อ้างถึง ServerConfig เดิม ซึ่งต้องมีอายุยาวกว่า result
base/relativePath เป็นข้อมูล route สำหรับ handler ไม่ใช่ใบรับรองว่าไฟล์ปลอดภัยแล้ว
handler ที่แก้ไขไฟล์หรือรัน CGI ต้องตรวจ containment ของ operation ตัวเอง
เพราะ path อาจเปลี่ยนระหว่าง request ได้ และ task นี้ทำ secure reads สำหรับ static content เท่านั้น

Router ไม่มี socket I/O จึงไม่เพิ่ม poll loop ที่สอง
อย่าเรียก send/read socket จาก Router; network layer ต้องจัดการ partial send,
disconnect, timeout, request size limits, headers ที่ระดับ connection และ keep-alive

ตอนนี้ static file/listing สร้าง body ทั้งก้อนใน memory
ไฟล์หรือ directory ขนาดใหญ่มากจึงใช้ RAM ตามขนาดข้อมูล
ยังไม่ใช่ streaming implementation และยังไม่พิสูจน์ load/stress ของ server เต็มระบบ
std::gmtime ใช้กับ event loop เดี่ยวตามแบบโปรเจกต์; หากเพิ่ม threads ต้องทบทวนส่วนนี้

## แผนที่ไฟล์และแนวทางอ่านเพื่ออธิบายเอง

| ไฟล์ | หน้าที่ |
|---|---|
| include/http/Router.hpp + src/http/Router.cpp | pipeline และ dispatch |
| PathResolver.hpp/.cpp | URL validation, encoding, descriptor ownership, secure reads |
| HttpResponse.hpp/.cpp | status/header/body และ HTTP serialization |
| MimeTypes.hpp/.cpp | extension -> Content-Type |
| AutoIndex.hpp/.cpp | directory entries -> escaped HTML |
| config/LocationConfig | alias, copy/assignment, validation และ trailing slash |
| config/ServerConfig.cpp | longest literal prefix |
| tests/router_demo.cpp | adapter สำหรับทดสอบโดยไม่ต้องมี server |
| tests/test_router.py | assertions ต่อ config + routing + filesystem + HTTP bytes |

แนะนำเริ่มอ่าน Router::route() ให้เข้าใจเส้นทางปกติก่อน
แล้วตามไป normalizeTarget(), openBeneath(), generate() ตามลำดับ

คำถามฝึกอธิบายก่อนตรวจงาน:

1. ทำไมต้อง decode ก่อนตรวจ .. แต่ห้าม decode สองครั้ง?
2. ทำไม root /srv/www ใน location /img/ จึงหา /srv/www/img/a.png?
3. ทำไมเช็ก string ว่าขึ้นต้นด้วย root อย่างเดียวกัน symlink ไม่ได้?
4. ทำไมใช้ descriptor แล้วจึง stat ผ่าน procfs?
5. ทำไม filename ที่ escape HTML แล้วยังต้อง encode สำหรับ href อีก?
6. ทำไม method ไม่อนุญาตต้องมี Allow แต่ method ที่อนุญาตแล้วส่งต่อ handler?
7. ทำไม HEAD ไม่ส่ง body แต่ Content-Length ยังต้องเท่ากับ GET?
8. ตอนนี้ผ่าน component tests แล้ว ยังขาดหลักฐานอะไรสำหรับ server เต็มระบบ?

## แหล่งอ้างอิงและการใช้ AI

- en.subject.pdf version 24.1 ใน repository
- [RFC 9110: HTTP semantics](https://www.rfc-editor.org/rfc/rfc9110.html) — method/status/Allow/HEAD
- [NGINX core module](https://nginx.org/en/docs/http/ngx_http_core_module.html) — location/root/alias
- [Linux proc_pid_fd](https://man7.org/linux/man-pages/man5/proc_pid_fd.5.html) — descriptor paths ใน procfs

AI ช่วยอ่านข้อกำหนด ออกแบบและเขียนโมดูล routing/static/AutoIndex,
ปรับ config integration, สร้าง tests และคู่มือนี้
ผู้เรียนควรเดินตามโค้ดและลองเปลี่ยนกรณีทดสอบเอง โดยเฉพาะ containment และการส่งต่อ handler
เอกสารนี้รายงานเฉพาะผลทดสอบที่รันจริง ไม่ถือว่าโปรเจกต์ Webserv ทั้งหมดเสร็จแล้ว
