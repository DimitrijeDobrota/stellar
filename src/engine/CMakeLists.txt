add_executable(engine
    engine.c
    score.c
    transposition.c
)

target_link_libraries(engine
    PRIVATE attacks
    PRIVATE board
    PRIVATE moves
    PRIVATE piece
    PRIVATE utils
    PRIVATE random
)

target_link_libraries(engine PRIVATE "cul")

set_target_properties(engine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
