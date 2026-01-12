#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "common.hpp"
#include "utils.hpp"

#undef unix

namespace byteo::unix {
    namespace stream {
        descriptor socket() {
            int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);
            uint64_t fingerprint = byteo::utils::random_u64(byteo::utils::mersenne);

            byteo::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;

            sock.family = AF_UNIX;
            sock.type = SOCK_STREAM;
            sock.sockaddr_size = sizeof(sockaddr_un);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(stream|unix): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }

    namespace datagram {
        descriptor socket() {
            int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);
            uint64_t fingerprint = byteo::utils::random_u64(byteo::utils::mersenne);

            byteo::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;

            sock.family = AF_UNIX;
            sock.type = SOCK_DGRAM;
            sock.sockaddr_size = sizeof(sockaddr_un);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(stream|unix): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }

    void unlink(std::string path) {
        ::unlink(path.c_str());
    }

    void unlink(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("close(unix): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        if (sock.listen) ::unlink(sock.laddress.string().c_str());
    }
}