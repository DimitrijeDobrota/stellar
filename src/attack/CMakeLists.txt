add_library(attack attack.cpp bishop.cpp rook.cpp)

target_link_libraries(attack
    PRIVATE bitboard
    PRIVATE utils
)

target_include_directories(attack INTERFACE .)
