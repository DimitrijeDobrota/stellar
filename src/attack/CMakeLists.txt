add_library(attack OBJECT
    attack.c
)

target_include_directories(attack
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
