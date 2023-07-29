add_library(moves OBJECT
    moves.c
)

target_include_directories(moves
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
