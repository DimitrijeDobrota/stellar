add_library(moves OBJECT
    moves.c
    moves_make.c
    moves_generate.c
)

target_include_directories(moves
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
