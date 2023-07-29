add_library(piece OBJECT
    piece.c
)

target_include_directories(piece
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
