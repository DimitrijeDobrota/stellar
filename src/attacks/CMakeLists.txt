add_library(attacks OBJECT
    attacks.c
)

target_include_directories(attacks
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
