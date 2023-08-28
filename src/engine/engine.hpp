#ifndef STELLAR_ENGINE_H
#define STELLAR_ENGINE_H

#include "engine/uci.hpp"
#include "uci.hpp"

namespace engine {
void search_position(const uci::Settings &setting);
}

#endif
