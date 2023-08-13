add_library(moves OBJECT
    move.cpp
    movelist.cpp
)

target_include_directories(moves
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
