#pragma once
#include <random>
#include <limits>
#include <stdexcept>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>
#endif

#include "def.hpp"
#include "socket.hpp"

#undef unix

namespace byteo {
    namespace utils {
        inline std::random_device rd; 
        inline std::mt19937_64 mersenne(rd());

        inline std::uniform_int_distribution<std::mt19937_64::result_type> random_s32(0, std::numeric_limits<int32_t>::max());
        inline std::uniform_int_distribution<std::mt19937_64::result_type> random_u64(0, std::numeric_limits<uint64_t>::max());

        inline int32_t find_free_id() {
            std::unique_lock lock(socket_table_mutex);

            while (true) {
                int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);

                if (socket_table.find(id) == socket_table.end()) return id;
            }
        }

        inline bool descriptor_ok(descriptor desc) {
            auto it = socket_table.find(desc.id);

            if (it == socket_table.end()) return false;

            return desc.fingerprint == it->second.fingerprint;
        }

        inline fd_t realfd(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("realfd(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            return sock.fd;
        }

        inline descriptor get_descriptor(fd_t fd) {
            std::unique_lock lock(socket_table_mutex);

            for (auto& [id, socket] : socket_table) if (socket.fd == fd) return {id, socket.fingerprint};

            return {};
        }

#ifdef _WIN32
        inline void wsainit() {
            std::unique_lock lock(wsa_mtx);

            if (!wsa_init) WSAStartup(MAKEWORD(2, 2), &wsa_data);
        }
#endif

        template<typename T>
        inline void setsockopt(descriptor desc, int32_t level, int32_t optname, T optval) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("setsockopt(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            if (::setsockopt(sock.fd, level, optname, reinterpret_cast<char*>(&optval), sizeof(T)) == -1) throw std::runtime_error("setsockopt(): Unable to set socket option: " + std::string(strerror(errno)));
        }

        template<>
        inline void setsockopt<bool>(descriptor desc, int32_t level, int32_t optname, bool optval) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("setsockopt(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            int32_t int_optval = optval;

            if (::setsockopt(sock.fd, level, optname, reinterpret_cast<char*>(&int_optval), sizeof(int32_t)) == -1) throw std::runtime_error("setsockopt(): Unable to set socket option: " + std::string(strerror(errno)));
        }

        template<typename T>
        inline int32_t getsockopt(descriptor desc, int32_t level, int32_t optname, T& optval, socklen_t size = sizeof(T)) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("getsockopt(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            if (::getsockopt(sock.fd, level, optname, &optval, &size) == -1) throw std::runtime_error("getsockopt(): Unable to get socket option: " + std::string(strerror(errno)));

            return size;
        }

        inline address getsockname(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("getsockname(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_storage my_addr{};
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getsockname(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getsockname(): Unable to get socket name: " + std::string(strerror(errno)));

            return address::from_sockaddr(my_addr);
        }

        inline address getpeername(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_storage my_addr{};
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getpeername(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getpeername(): Unable to get socket name: " + std::string(strerror(errno)));

            return address::from_sockaddr(my_addr);
        }

        inline uint32_t tcpreadavailable(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(): socket closed");

            byteo::utils::socket& sock = socket_table.at(desc.id);

            uint32_t bytes_available;
#ifdef _WIN32
            ioctlsocket(sock.fd, FIONREAD, reinterpret_cast<u_long*>(&bytes_available));
#else
            ioctl(sock.fd, FIONREAD, &bytes_available);
#endif
            return bytes_available;
        }
    }
}