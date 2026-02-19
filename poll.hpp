#pragma once
#include <vector>
#include <mutex>
#include <cstdint>

#ifdef __linux__
#include <sys/poll.h>
#endif

#ifdef _WIN32
#include "windowsheader.hpp"
#include <winsock2.h>
#endif

#include "def.hpp"
#include "utils.hpp"

namespace byteo {
    class poller {
        std::mutex poll_mtx;
        std::vector<pollfd> fds;

        int32_t find(descriptor desc) {
            fd_t fd = utils::realfd(desc);

            for (int32_t i = 0; i < fds.size(); i++) if (fds[i].fd == fd) return i;

            return -1;
        }
    public:
        void add(descriptor desc, int16_t events) {
            std::unique_lock lock(poll_mtx);

            fds.push_back({utils::realfd(desc), events, 0});
        }

        void modify(descriptor desc, int16_t events) {
            std::unique_lock lock(poll_mtx);

            int32_t index = find(desc);

            if (index >= 0) fds[index].events = events;
        }

        void remove(descriptor desc) {
            std::unique_lock lock(poll_mtx);

            int32_t index = find(desc);

            if (index >= 0) fds.erase(fds.begin() + index);
        }

        void removeAll() {
            std::unique_lock lock(poll_mtx);

            fds.clear();
        }

        std::vector<event> poll(int32_t timeout = -1) {
            std::unique_lock lock(poll_mtx);

            std::vector<pollfd> fds_copy = fds;

            lock.unlock();

            int32_t event_num = ::poll(fds_copy.data(), fds_copy.size(), timeout);

            std::vector<event> ret;

            if (event_num) {
                int32_t n_events = 0;

                for (int32_t i = 0; i < fds_copy.size() && n_events < event_num; i++) {
                    pollfd& pfd = fds_copy[i];

                    if (pfd.revents) {
                        ret.push_back({utils::get_descriptor(pfd.fd), pfd.revents});

                        n_events++;
                        pfd.revents = 0;
                    }
                }
            }

            return ret;
        }
    };
}