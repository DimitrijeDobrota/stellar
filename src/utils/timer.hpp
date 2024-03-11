#ifndef STELLAR_TIME_H
#define STELLAR_TIME_H

#include <chrono>

namespace timer {

inline uint32_t get_ms() {
    const auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

} // namespace timer

#endif
