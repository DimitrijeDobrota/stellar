add_library(utils OBJECT
    utils.c
)

target_include_directories(utils
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)

add_library(random OBJECT
    random.c
)

target_include_directories(random
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
