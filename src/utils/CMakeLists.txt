add_library(random OBJECT
    random.cpp
)

target_include_directories(random
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
)
