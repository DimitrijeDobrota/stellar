add_library(board OBJECT
    board.c
    zobrist.c
)

target_include_directories(board
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
