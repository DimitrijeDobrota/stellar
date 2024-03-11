add_library(attack attack.cpp bishop.cpp rook.cpp)

target_link_libraries(attack
    PRIVATE bit
    PRIVATE bitboard
    PRIVATE color
    PRIVATE square
    PRIVATE utils
)

target_include_directories(attack INTERFACE .)

stellar_target_precompile_headers(attack
    PRIVATE "bishop.hpp"
    PRIVATE "king.hpp"
    PRIVATE "knight.hpp"
    PRIVATE "pawnb.hpp"
    PRIVATE "pawnw.hpp"
    PRIVATE "queen.hpp"
    PRIVATE "rook.hpp"
    PRIVATE "slider.hpp"
)
