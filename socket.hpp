#pragma once
#include <map>
#include <mutex>
#include <atomic>
#include <cstdint>

#include "def.hpp"
#include "address.hpp"

namespace byteo {
    namespace utils {
        struct socket {
            fd_t fd;

            uint64_t fingerprint;

            std::atomic_bool working;
            std::atomic_bool blocking;
            std::atomic_bool listen;
            std::atomic_bool accepted;

            int32_t family;
            int32_t type;
            int32_t sockaddr_size;

            address laddress;
            address raddress;

            std::mutex readMtx;
            std::mutex writeMtx;
        };
    }

    inline std::map<int32_t, utils::socket> socket_table;
    inline std::recursive_mutex socket_table_mutex;

#ifdef _WIN32
    std::mutex wsa_mtx;

    bool wsa_init;

    WSAData wsa_data;
#endif
}