#pragma once
#include <string>
#include <array>
#include <variant>
#include <stdexcept>
#include <cstdint>
#include <cstring>

#ifdef __linux__
#include <sys/un.h>
#include <netinet/in.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace byteo {
    class address {
#ifdef __linux__
        using raw_address = std::array<uint8_t, sizeof(sockaddr_un::sun_path)>;

        std::variant<raw_address, in_addr, in6_addr> __addr;
#endif

#ifdef _WIN32
        std::variant<in_addr, in6_addr> __addr;
#endif

        uint16_t __port;
        uint16_t __family;
    public:
        address() : __port(0), __family(AF_UNSPEC) {}

        address(const address& addr) {
            __addr = addr.__addr;
            __port = addr.__port;
            __family = addr.__family;
        }

        // address(SAType __type) : __addr_type(__type) {}
        address(sockaddr addr) {
            if (addr.sa_family == AF_INET) address(*reinterpret_cast<sockaddr_in*>(&addr));
        }

        address(sockaddr_in addr) {
            __addr = addr.sin_addr;
            __port = ntohs(addr.sin_port);
            __family = addr.sin_family;
        }

        address(sockaddr_in6 addr) {
            __addr = addr.sin6_addr;
            __port = ntohs(addr.sin6_port);
            __family = addr.sin6_family;
        }

#ifdef __linux__
        address(sockaddr_un addr) {
            __family = addr.sun_family;

            std::get<raw_address>(__addr).fill(0);
            std::copy(addr.sun_path, addr.sun_path + sizeof(raw_address), std::get<raw_address>(__addr).begin());
        }
#endif

        address(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint16_t p) : __port(p), __family(AF_INET) {
            __addr = in_addr{a1 | (a2 << 8) | (a3 << 16) | (a4 << 24)};
        }

        address(uint32_t addr) : __family(AF_INET) {
            __addr = in_addr{addr};
        }

        address(uint32_t addr, uint16_t p) : __port(p), __family(AF_INET) {
            __addr = in_addr{addr};
        }

#ifdef __linux__
        address(std::string unix_addr) : __family(AF_UNIX) {
            std::get<raw_address>(__addr).fill(0);
            std::copy(unix_addr.begin(), unix_addr.end(), std::get<raw_address>(__addr).begin());
        }
#endif

        address& operator=(const address& addr) {
            __addr = addr.__addr;
            __port = addr.__port;
            __family = addr.__family;

            return *this;
        }

        address& operator=(sockaddr_in addr) {
            __addr = addr.sin_addr;
            __port = ntohs(addr.sin_port);
            __family = addr.sin_family;

            return *this;
        }

        address& operator=(sockaddr_in6 addr) {
            __addr = addr.sin6_addr;
            __port = ntohs(addr.sin6_port);
            __family = addr.sin6_family;

            return *this;
        }

#ifdef __linux__
        address& operator=(sockaddr_un addr) {
            __family = addr.sun_family;

            std::get<raw_address>(__addr).fill(0);
            std::copy(addr.sun_path, addr.sun_path + sizeof(raw_address), std::get<raw_address>(__addr).begin());
            
            return *this;
        }
#endif

        address& operator=(uint32_t addr) {
            __addr = in_addr{addr};

            return *this;
        }

#ifdef __linux__
        address& operator=(std::string unix_addr) {
            std::get<raw_address>(__addr).fill(0);
            std::copy(unix_addr.begin(), unix_addr.end(), std::get<raw_address>(__addr).begin());

            return *this;
        }
#endif

        static address from_sockaddr(sockaddr_storage addr) {
            if (addr.ss_family == AF_INET) return address(*reinterpret_cast<sockaddr_in*>(&addr));
            if (addr.ss_family == AF_INET6) return address(*reinterpret_cast<sockaddr_in6*>(&addr));

#ifdef __linux__
            if (addr.ss_family == AF_UNIX) return address(*reinterpret_cast<sockaddr_un*>(&addr));
#endif

            else throw std::invalid_argument("Unsupported address family");
        }

        void family(int16_t f) {
            __family = f;
        }

        int16_t family() {
            return __family;
        }

        void port(int16_t p) {
            __port = p;
        }

        int16_t port() {
            return __port;
        }

        uint32_t IPv4() {
            return (__family == AF_INET) ? std::get<in_addr>(__addr).s_addr : 0;
        }

        sockaddr_in addrInet() {
            if (__family != AF_INET) return {0};

            sockaddr_in addr{};
            addr.sin_addr = std::get<in_addr>(__addr);
            addr.sin_port = htons(__port);
            addr.sin_family = __family;

            return addr;
        }

        sockaddr_in6 addrInet6() {
            if (__family != AF_INET6) return {0};

            sockaddr_in6 addr{};
            addr.sin6_addr = std::get<in6_addr>(__addr);
            addr.sin6_port = htons(__port);
            addr.sin6_family = __family;

            return addr;
        }
        
#ifdef __linux__
        sockaddr_un addrUnix() {
            if (__family != AF_UNIX) return {0};

            sockaddr_un addr{};
            addr.sun_family = __family;

            std::copy(std::get<raw_address>(__addr).begin(), std::get<raw_address>(__addr).end(), addr.sun_path);

            return addr;
        }
#endif

        sockaddr_storage addr() {
            sockaddr_storage storage{};

            if (__family == AF_INET) {
                sockaddr_in addr = addrInet();

                std::memcpy(&storage, &addr, sizeof(addr));
            }
            
            else if (__family == AF_INET6) {
                sockaddr_in6 addr = addrInet6();

                std::memcpy(&storage, &addr, sizeof(addr));
            }

#ifdef __linux__
            else if (__family == AF_UNIX) {
                sockaddr_un addr = addrUnix();

                std::memcpy(&storage, &addr, sizeof(addr));
            }
#endif

            return storage;
        }

        std::string string() {
            std::string str_addr;

            if (__family == AF_INET) {
                uint32_t addr = ntohl(std::get<in_addr>(__addr).s_addr);

                str_addr = std::to_string((addr >> 24) & 0xFF) + "." +
                        std::to_string((addr >> 16) & 0xFF) + "." +
                        std::to_string((addr >> 8) & 0xFF) + "." +
                        std::to_string(addr & 0xFF) + ":" +
                        std::to_string(__port);
            }

            else if(__family == AF_INET6) {}

#ifdef __linux__
            else if(__family == AF_UNIX) str_addr = std::string(reinterpret_cast<char*>(std::get<raw_address>(__addr).data()));
#endif

            return str_addr;
        }

        operator uint32_t() {
            return IPv4();
        }

        operator sockaddr_in() {
            return addrInet();
        }

        operator sockaddr_in6() {
            return addrInet6();
        }

#ifdef __linux__
        operator sockaddr_un() {
            return addrUnix();
        }
#endif

        operator sockaddr_storage() {
            return addr();
        }

        operator std::string() {
            return string();
        }
    };

    inline const address nulladdr;
}