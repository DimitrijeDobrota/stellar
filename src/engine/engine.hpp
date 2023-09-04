#ifndef STELLAR_ENGINE_H
#define STELLAR_ENGINE_H

#include "engine/uci.hpp"
#include "move.hpp"
#include "uci.hpp"

namespace engine {

Move search_position(const uci::Settings &setting);

class PVTable;
std::ostream &operator<<(std::ostream &os, const PVTable &pvtable);

} // namespace engine

#endif
