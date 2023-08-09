add_library(board OBJECT
    board.cpp
    zobrist.cpp
)

target_include_directories(board
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
