add_library(utils OBJECT
    utils.c
)

target_include_directories(utils
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
