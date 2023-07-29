add_executable(perft perft.c)

target_link_libraries(perft
    PRIVATE attack
    PRIVATE board
    PRIVATE moves
    PRIVATE score
    PRIVATE utils
)

target_link_libraries(perft PRIVATE "cul")

set_target_properties(perft PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
