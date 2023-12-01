#ifndef STELLAR_TIME_H
#define STELLAR_TIME_H

#include <sys/time.h>

namespace timer {

inline uint32_t get_ms() {
    struct timeval time{};
    gettimeofday(&time, nullptr);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

} // namespace timer

#endif
