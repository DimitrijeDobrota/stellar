add_executable(engine
    engine.cpp
    evaluate.cpp
    uci.cpp
)

option(STELLAR_STATS "Provide stats for analysis" OFF)
if(STELLAR_STATS)
    add_definitions(-DUSE_STATS)
endif()

target_link_libraries(engine
    PRIVATE Stellar_version
    PRIVATE bitboard
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
