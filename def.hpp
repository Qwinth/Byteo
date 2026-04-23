#pragma once
#include <vector>
#include <cstdint>

#include "address.hpp"

#undef unix

namespace byteo {
    using fd_t = int32_t;
#ifdef __linux__
    using socklen_t = uint32_t;
#endif

#ifdef _WIN32
    using socklen_t = int32_t;
#endif
    using address_list = std::vector<address>;

    struct descriptor {
        int32_t id;
        uint64_t fingerprint;
    };

    struct event {
        descriptor desc;
        int16_t mask;
    };

    struct dataless_datagram {
        address addr;
        int64_t size;
    };

    struct datagram {
        address addr;
        std::vector<std::byte> data;
    };

    struct string_datagram {
        address addr;
        std::string data;
    };

    inline bool operator==(const descriptor& obj1, const descriptor& obj2) {
        return obj1.id == obj2.id && obj1.fingerprint == obj2.fingerprint;
    }

    inline bool operator!=(const descriptor& obj1, const descriptor& obj2) {
        return !(obj1 == obj2);
    }
}