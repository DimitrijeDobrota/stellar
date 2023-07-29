add_executable(engine engine.c)

target_link_libraries(engine
    PRIVATE attack
    PRIVATE board
    PRIVATE moves
    PRIVATE score
    PRIVATE utils
)

target_link_libraries(engine PRIVATE "cul")

set_target_properties(engine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
