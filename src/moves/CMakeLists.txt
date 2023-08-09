add_library(moves OBJECT
    moves.cpp
    moves_make.cpp
    moves_generate.cpp
)

target_include_directories(moves
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
