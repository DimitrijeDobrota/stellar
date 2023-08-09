#include "piece.hpp"

#include <iostream>
int main(void) {
    std::cout << piece::get(piece::Type::PAWN, Color::WHITE).code << std::endl;
}
