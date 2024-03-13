add_library(attack attack.cpp bishop.cpp rook.cpp)

target_link_libraries(attack
    PRIVATE bitboard
    PRIVATE utils
    PRIVATE piece
)

target_include_directories(attack INTERFACE .)
