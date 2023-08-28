add_library(board OBJECT
    board.cpp
)

target_link_libraries(board
    PRIVATE utils
    PUBLIC piece
    PUBLIC random
)

target_include_directories(board INTERFACE .)
stellar_target_precompile_headers(board INTERFACE "board.hpp")


