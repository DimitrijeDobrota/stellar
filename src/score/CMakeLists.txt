add_library(score OBJECT
    score.c
)

target_include_directories(score
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
