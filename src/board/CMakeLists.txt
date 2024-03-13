add_library(board OBJECT
    board.cpp
)

target_link_libraries(board
    PRIVATE bitboard
    PRIVATE utils
    PRIVATE piece
    PUBLIC random
)

target_include_directories(board INTERFACE .)
