#pragma once
#include <vector>
// #include <map>
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

    // enum class result_code {
    //     ok,
    //     retry,
    //     socket_closed,
    //     invalid_argument,

    // };

    template<class T>
    class result {
    public:
        result(const T& obj) : data(obj) {}
        result(const T& obj, int32_t status) : data(obj), code(status) {}
        // result(const T& obj, result_code status, const std::string& what) : data(obj), code(status), what(what) {}

        int32_t status_code() { return code; }
        bool ok() { return !code; }

        T& value() { return data; }

        operator T&() { return data; }
    
    private:
        T data{};

        int32_t code = 0;
        // result_code code = result_code::ok;
        // std::string what;
    };

    // inline std::map<int32_t, result_code> errno2code = {{0, result_code::ok}, {EAGAIN, result_code::retry}, {EWOULDBLOCK, result_code::retry}};

    inline bool operator==(const descriptor& obj1, const descriptor& obj2) {
        return obj1.id == obj2.id && obj1.fingerprint == obj2.fingerprint;
    }

    inline bool operator!=(const descriptor& obj1, const descriptor& obj2) {
        return !(obj1 == obj2);
    }
}