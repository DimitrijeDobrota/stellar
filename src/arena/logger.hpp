#ifndef STELLAR_ARENA_LOGGER_H
#define STELLAR_ARENA_LOGGER_H

#include "utils.hpp"
#include <cerrno>
#include <cstring>
#include <format>
#include <iostream>

namespace logger {

enum Level {
    Critical,
    Arena,
    Debug,
    Info,
};

static Level active = Debug;
inline void set_level(const Level lvl) { active = lvl; }

inline void log(const std::string &message, const Level lvl = Arena) {
    static const std::string name[] = {"crit", "arena", "debug", "info"};
    if (lvl > active) return;
    std::cerr << std::format("[ {:>5} ] {}\n", name[lvl], message);
}

inline void error(const char *call) { log(std::format("{}, {}", call, std::strerror(errno)), Critical); }

} // namespace logger

#endif
