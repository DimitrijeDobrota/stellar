add_library(attacks OBJECT
    attacks.cpp
    internal.cpp
    magic.cpp
)

target_include_directories(attacks
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
    PRIVATE "internal"
)


project(
    MagicGenerator
    VERSION 1.0.0
    DESCRIPTION "Generator of magic numbers for bishup and rook attacks"
    LANGUAGES CXX
)


add_executable(magic
    internal.cpp
    magic.cpp
    magic_generate.cpp
)

target_link_libraries(magic
    PRIVATE random
)

set_target_properties(magic PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
