add_executable(engine
    engine.cpp
    uci.cpp
)

target_link_libraries(engine
    PRIVATE Stellar_version
    PRIVATE board
    PRIVATE moves
    PRIVATE piece
    PRIVATE utils
)

set_target_properties(engine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
