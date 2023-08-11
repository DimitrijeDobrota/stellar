add_executable(engine
    engine.cpp
    score.cpp
    transposition.cpp
)

target_link_libraries(engine
    PRIVATE board
    PRIVATE moves
    PRIVATE random
)

target_link_libraries(engine PRIVATE "cul")

set_target_properties(engine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
