#ifndef STELLAR_ENGINE_H
#define STELLAR_ENGINE_H

#include "engine/uci.hpp"
#include "move.hpp"
#include "uci.hpp"

namespace engine {

Move search_position(const uci::Settings &setting);

}

#endif
