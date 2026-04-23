#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <cstddef>
#include <cstdint>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>

#define SHUT_RDWR SD_BOTH
#endif

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "utils.hpp"

namespace byteo {
    inline void connect(descriptor desc, address addr) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("connect(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("connect(): Invalid address family");

        sockaddr_storage tmp_addr = addr;

        if (::connect(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("connect(): Unable to connect to host: " + std::string(strerror(errno)));

        sock.working = true;
        sock.laddress = byteo::utils::getsockname(desc);
        sock.raddress = addr;
    }

    inline void connect(descriptor desc, address_list addr_list) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("connect(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        for (address& addr : addr_list) {
            if (addr.family() != sock.family) continue;
            
            sockaddr_storage tmp_addr = addr;

            if (::connect(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) continue;

            sock.working = true;
            sock.laddress = byteo::utils::getsockname(desc);
            sock.raddress = addr;

            break;
        }

        if (!sock.working) throw std::runtime_error("connect(): Unable to connect to host: " + std::string(strerror(errno)));
    }

    inline void bind(descriptor desc, address addr) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("bind(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("bind(): Invalid address family");

        sockaddr_storage tmp_addr = addr;

        if (::bind(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("bind(): Unable to connect to host: " + std::string(strerror(errno)));

        sock.working = true;
        sock.laddress = addr;
    }

    inline void bind(descriptor desc, address_list addr_list) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("bind(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        for (address& addr : addr_list) {
            if (addr.family() != sock.family) continue;

            sockaddr_storage tmp_addr = addr;

            if (::bind(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("bind(): Unable to connect to host: " + std::string(strerror(errno)));

            sock.working = true;
            sock.laddress = addr;

            break;
        }

        if (!sock.working) throw std::runtime_error("bind(): Unable to bind to host: "  + std::string(strerror(errno)));
    }

    inline descriptor accept(descriptor desc) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("accept(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        sockaddr_storage addr{};
        socklen_t socklen = sock.sockaddr_size;

        int32_t new_fd = ::accept(sock.fd, reinterpret_cast<sockaddr*>(&addr), &socklen);

        if (new_fd == -1) throw std::runtime_error("accept(): Unable to accept connection: " + std::string(strerror(errno)));

        int32_t id = byteo::utils::random_s32(byteo::utils::mersenne);
        uint64_t fingerprint = byteo::utils::random_u64(byteo::utils::mersenne);

        descriptor new_desc = {id, fingerprint};

        byteo::utils::socket& new_sock = socket_table.try_emplace(id).first->second;

        new_sock.fd = new_fd;
        new_sock.fingerprint = fingerprint;

        new_sock.working = true;
        new_sock.blocking = true;
        new_sock.listen = false;
        new_sock.accepted = true;

        new_sock.family = AF_INET;
        new_sock.type = sock.family;
        new_sock.sockaddr_size = sock.sockaddr_size;

        new_sock.laddress = byteo::utils::getsockname(new_desc);
        new_sock.raddress = address::from_sockaddr(addr);

        return new_desc;
    }

    inline void listen(descriptor desc, int32_t __listen) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("listen(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        if (::listen(sock.fd, __listen) == -1) throw std::runtime_error("listen(): Unable to listen to host: " + std::string(strerror(errno)));

        sock.listen = true;
    }

    inline int64_t read(descriptor desc, void* buffer, int64_t size, int32_t flags = 0) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        fd_t fd = sock.fd;

        tableLock.unlock();
        std::unique_lock readLock(sock.readMtx);

        int64_t read_size = ::recv(fd, reinterpret_cast<char*>(buffer), size, flags);

        return read_size;
    }

    inline std::vector<std::byte> read(descriptor desc, int64_t size, int32_t flags = 0) {
        std::vector<std::byte> buffer(size);
        buffer.resize(read(desc, buffer.data(), size, flags));

        return buffer;
    }

    template<typename blob_type>
    inline blob_type readblob(descriptor desc, int32_t flags = 0) {
        blob_type blob;

        if (read(desc, &blob, sizeof(blob_type), flags) < sizeof(blob_type)) throw std::runtime_error("readblob(): corrupted blob");

        return blob;
    }

    inline std::string readstring(descriptor desc, int64_t size, int32_t flags = 0) {
        std::string buffer(size, 0);
        buffer.resize(read(desc, buffer.data(), size, flags));

        return buffer;
    }

    inline int64_t write(descriptor desc, const void* buffer, int64_t size, int32_t flags = 0) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        fd_t fd = sock.fd;

        tableLock.unlock();
        std::unique_lock writeLock(sock.writeMtx);

        return ::send(fd, reinterpret_cast<const char*>(buffer), size, flags);
    }

    inline int64_t write(descriptor desc, const std::vector<std::byte>& buffer, int32_t flags = 0) {
        return write(desc, buffer.data(), buffer.size(), flags);
    }

    template<typename blob_type>
    inline int64_t writeblob(descriptor desc, const blob_type& blob, int32_t flags = 0) {
        return write(desc, &blob, sizeof(blob_type), flags);
    }

    inline int64_t writestring(descriptor desc, const std::string& string, int32_t flags = 0) {
        return write(desc, string.c_str(), string.size(), flags);
    }

    inline dataless_datagram readfrom(descriptor desc, void* buffer, int64_t size, int32_t flags = 0) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("readfrom(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        fd_t fd = sock.fd;

        tableLock.unlock();
        std::unique_lock readLock(sock.readMtx);

        sockaddr_storage tmp_addr{};
        socklen_t socklen = sock.sockaddr_size;

        int64_t read_size = ::recvfrom(fd, reinterpret_cast<char*>(buffer), size, flags, reinterpret_cast<sockaddr*>(&tmp_addr), &socklen);

        if (!socklen) return {nulladdr, read_size};

        return {address::from_sockaddr(tmp_addr), read_size};
    }

    inline datagram readfrom(descriptor desc, int64_t size, int32_t flags = 0) {
        std::vector<std::byte> buffer(size);

        dataless_datagram tmp = readfrom(desc, buffer.data(), size, flags);
        buffer.resize(tmp.size);

        return {tmp.addr, buffer};
    }

    inline string_datagram readstringfrom(descriptor desc, int64_t size, int32_t flags = 0) {
        std::string buffer(size, 0);
        dataless_datagram read = readfrom(desc, buffer.data(), size, flags);

        buffer.resize(read.size);

        return {read.addr, buffer};
    }

    inline int64_t writeto(descriptor desc, const void* buffer, int64_t size, address addr, int32_t flags = 0) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("writeto(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        fd_t fd = sock.fd;

        tableLock.unlock();
        std::unique_lock writeLock(sock.writeMtx);

        sockaddr_storage tmp_addr = addr;

        return ::sendto(fd, reinterpret_cast<const char*>(buffer), size, flags, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size);
    }

    inline int64_t writeto(descriptor desc, const std::vector<std::byte>& buffer, address addr, int32_t flags = 0) {
        return writeto(desc, buffer.data(), buffer.size(), addr, flags);
    }

    inline int64_t writestringto(descriptor desc, const std::string& string, address addr, int32_t flags = 0) {
        return writeto(desc, string.c_str(), string.size(), addr, flags);
    }

    inline void shutdown(descriptor desc) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("shutdown(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

        ::shutdown(sock.fd, SHUT_RDWR);

        sock.working = false;
    }

    inline void close(descriptor desc) {
        std::unique_lock tableLock(socket_table_mutex);

        if (!byteo::utils::descriptor_ok(desc)) throw std::runtime_error("close(): socket closed");

        byteo::utils::socket& sock = socket_table.at(desc.id);

#ifdef __linux__
        ::close(sock.fd);
#endif

#ifdef _WIN32
        ::closesocket(sock.fd);
#endif
        
        std::unique_lock readLock(sock.readMtx);
        std::unique_lock writeLock(sock.writeMtx);

        readLock.unlock();
        writeLock.unlock();

        socket_table.erase(desc.id);
    }
}