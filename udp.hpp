#pragma once
#include <stdexcept>
#include <mutex>

#ifdef __linux__
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>
#endif

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "common.hpp"
#include "utils.hpp"

namespace byteo {
    namespace ipv4::udp {
        descriptor socket() {
#ifdef _WIN32
            byteo::utils::wsainit();
#endif

            int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);
            uint64_t fingerprint = byteo::utils::random_u64(byteo::utils::mersenne);

            byteo::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_INET, SOCK_DGRAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;
            sock.accepted = false;

            sock.family = AF_INET;
            sock.type = SOCK_DGRAM;
            sock.sockaddr_size = sizeof(sockaddr_in);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(udp|ipv4): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }

    namespace ipv6::udp {
        descriptor socket() {
#ifdef _WIN32
            byteo::utils::wsainit();
#endif

            int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);
            uint64_t fingerprint = byteo::utils::random_u64(byteo::utils::mersenne);

            byteo::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_INET6, SOCK_DGRAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;
            sock.accepted = false;

            sock.family = AF_INET6;
            sock.type = SOCK_DGRAM;
            sock.sockaddr_size = sizeof(sockaddr_in6);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(udp|ipv6): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }
}