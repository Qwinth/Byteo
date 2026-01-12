 # Byteo

 **Byteo** — header-only C++ socket library supporting TCP, UDP, UNIX sockets, and SSL/TLS.
 It is designed for simplicity, clarity, and low-level control without losing elegance.

 ---

 ## Features

 - IPv4, IPv6, and UNIX socket support
 - TCP and UDP protocols
 - SSL/TLS support via OpenSSL
 - Unified API with clear namespace structure
 - Built-in DNS resolver
 - Simple option handling via `setsockopt`

 ---

 ## Installation

 No external dependencies except OpenSSL (for SSL/TLS).
 Just include the headers in your project:

 ```cpp
 #include "byteo/socket.hpp"
 #include "byteo/tcp.hpp"
 #include "byteo/udp.hpp"
 #include "byteo/unix.hpp"
 #include "byteo/common.hpp"
 #include "byteo/ssl.hpp"
 #include "byteo/dns.hpp"
 #include "byteo/utils.hpp"
 ```

 ---

 ## Examples

 ### TCP Server (IPv4)

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"

 int main() {
     byteo::descriptor sock = byteo::ipv4::tcp::socket();
     byteo::bind(sock, byteo::ipv4::dns::resolve("localhost", 8000));
     byteo::listen(sock, 0);

     while (true) {
         byteo::descriptor client = byteo::accept(sock);

         std::cout << byteo::readstring(client, 1024) << std::endl;
         byteo::writestring(client, "Hello from TCP server!");

         byteo::shutdown(client);
         byteo::close(client);
     }

     byteo::close(sock);
 }
 ```

 ---

 ### TCP Client (IPv4)

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 
 int main() {
     byteo::descriptor sock = byteo::ipv4::tcp::socket();
     byteo::connect(sock, byteo::ipv4::dns::resolve("localhost", 8000));
 
     byteo::writestring(sock, "Hi, server!");
     std::cout << byteo::readstring(sock, 1024) << std::endl;
 
     byteo::shutdown(sock);
     byteo::close(sock);
 }
 ```

 ---

 ### UNIX Socket Server

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "unix.hpp"
 
 int main() {
     byteo::descriptor sock = byteo::unix::socket();
     byteo::unix::unlink("test.socket");
     byteo::bind(sock, {"test.socket"});
     byteo::listen(sock, 0);
 
     while (true) {
         byteo::descriptor cl = byteo::accept(sock);
 
         std::cout << byteo::readstring(cl, 1024) << std::endl;
         byteo::writestring(cl, "Hello from UNIX server!");
         
         byteo::close(cl);
     }
 
     byteo::close(sock);
 }
 ```

 ---

 ### SSL/TLS Server

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 #include "ssl.hpp"
 
 int main() {
     byteo::utils::ssl::init();
     byteo::ssl_ctx ctx = byteo::utils::ssl::new_server_context();
 
     byteo::utils::ssl::load_cert(ctx, "server.crt");
     byteo::utils::ssl::load_key(ctx, "server.key");
 
     byteo::descriptor sock = byteo::ipv4::tcp::socket();
     byteo::bind(sock, byteo::ipv4::dns::resolve("localhost", 8000));
     byteo::listen(sock, 0);
 
     while (true) {
         byteo::descriptor cl = byteo::accept(sock);
 
         byteo::ssl::enable(cl, ctx);
         byteo::ssl::handshake(cl);
 
         std::cout << byteo::ssl::readstring(cl, 1024) << std::endl;
         byteo::ssl::writestring(cl, "Hello from SSL server!");
 
         byteo::ssl::shutdown(cl);
         byteo::close(cl);
     }
 
     byteo::close(sock);
 }
 ```

 ---

 ### SSL/TLS Client

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 #include "ssl.hpp"
 
 int main() {
     byteo::utils::ssl::init();
     byteo::ssl_ctx ctx = byteo::utils::ssl::new_client_context();
 
     byteo::descriptor sock = byteo::ipv4::tcp::socket();
     byteo::connect(sock, byteo::ipv4::dns::resolve("localhost", 8000));
 
     byteo::ssl::enable(sock, ctx);
     byteo::ssl::handshake(sock);
 
     byteo::ssl::writestring(sock, "Secure hello!");
     std::cout << byteo::ssl::readstring(sock, 1024) << std::endl;
 
     byteo::ssl::shutdown(sock);
     byteo::close(sock);
 }
 ```

---

 ### UDP Server

 ```cpp
 #include <iostream>
 #include <algorithm>
 #include "socket.hpp"
 #include "udp.hpp"
 #include "dns.hpp"
 
 int main() {
     byteo::descriptor sock = byteo::ipv4::udp::socket();
     byteo::bind(sock, byteo::ipv4::dns::resolve("localhost", 8000));
 
     while (true) {
         byteo::string_datagram temp = byteo::readstringfrom(sock, 1024);
 
         std::cout << temp.addr.string() << ": " << temp.data << std::endl;
 
         std::transform(temp.data.begin(), temp.data.end(), temp.data.begin(), ::toupper);
         
         byteo::writestringto(sock, temp.data, temp.addr);
     }
 
     byteo::close(sock);
 }
 ```

 ---

 ### UDP Client

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "udp.hpp"
 #include "dns.hpp"
 
 int main() {
     byteo::descriptor sock = byteo::ipv4::udp::socket();
     byteo::address_list addr = byteo::ipv4::dns::resolve("localhost", 8000);
 
     byteo::writestringto(sock, "Yay! UDP is working!", addr.front());
 
     byteo::string_datagram temp = byteo::readstringfrom(sock, 1024);
 
     std::cout << temp.addr.string() << ": " << temp.data << std::endl;
 
     byteo::close(sock);
 }
 ```

 ---

 ## DNS Example

 ```cpp
 auto addr = byteo::ipv4::dns::resolve("example.com", 443);
 ```

 ---

 ## Roadmap

 - UDP support ✅
 - Unified API for both stream and datagram sockets ✅
 - Add more error checking

 ---

 ## License

 MIT License
