add_library(moves OBJECT
    move.cpp
    movelist.cpp
)

target_link_libraries(moves
    PRIVATE bitboard
    PRIVATE board
    PRIVATE piece
    PRIVATE utils
)

target_include_directories(moves INTERFACE .)
