add_library(attacks OBJECT
    attacks.c
    internal.c
    magic.c
)

target_include_directories(attacks
    PUBLIC "${PROJECT_SOURCE_DIR}/src/include"
    PRIVATE "internal"
)


project(
    MagicGenerator
    VERSION 1.0.0
    DESCRIPTION "Generator of magic numbers for bishup and rook attacks"
    LANGUAGES C
)


add_executable(magic
    internal.c
    magic.c
    magic_generate.c
)

target_link_libraries(magic
    PRIVATE utils
)

set_target_properties(magic PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
