add_executable(arena
    arena.cpp
    match.cpp
    engine.cpp
    game.cpp
)

target_link_libraries(arena
    PRIVATE Stellar_version
    PRIVATE board
    PRIVATE moves
    PRIVATE piece
    PRIVATE utils
)

set_target_properties(arena PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
