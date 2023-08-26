add_library(moves OBJECT
    move.cpp
    movelist.cpp
)

target_link_libraries(moves
    PRIVATE piece
    PRIVATE board
    PRIVATE utils
)

target_include_directories(moves INTERFACE .)
