#pragma once
#include <string>
#include <stdexcept>
#include <cstdint>

#ifdef __linux__
#include <netdb.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>
#endif

#include "address.hpp"
#include "def.hpp"

namespace byteo {
    namespace ipv4::dns {
        address_list resolve(const std::string& host, uint16_t port) {
            address_list al;

            addrinfo hints {};
            addrinfo* result;

            hints.ai_family = AF_INET;

            int32_t status = getaddrinfo(host.c_str(), nullptr, &hints, &result);

            if (status != 0) throw std::runtime_error("resolve(ipv4): Unable to resolve host: " + std::string(gai_strerror(status)));

            for (addrinfo* res = result; res != nullptr; res = res->ai_next) {
                address addr = *reinterpret_cast<sockaddr_in*>(res->ai_addr);
                addr.port(port);

                al.push_back(addr);
            }

            freeaddrinfo(result);

            return al;
        }
    }

    namespace ipv6::dns {
        address_list resolve(const std::string& host, uint16_t port) {
            address_list al;

            addrinfo hints {};
            addrinfo* result;

            hints.ai_family = AF_INET6;

            int32_t status = getaddrinfo(host.c_str(), nullptr, &hints, &result);

            if (status != 0) throw std::runtime_error("resolve(ipv6): Unable to resolve host: " + std::string(gai_strerror(status)));

            for (addrinfo* res = result; res != nullptr; res = res->ai_next) {
                address addr = *reinterpret_cast<sockaddr_in6*>(res->ai_addr);
                addr.port(port);

                al.push_back(addr);
            }

            freeaddrinfo(result);

            return al;
        }
    }
}