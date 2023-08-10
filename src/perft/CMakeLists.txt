add_executable(perft perft.cpp)

option(WITH_FULL_COUNT "Make count on types of moves" OFF)
if(WITH_FULL_COUNT)
    add_definitions(-DUSE_FULL_COUNT)
endif()

target_link_libraries(perft
    PRIVATE attacks
    PRIVATE board
    PRIVATE moves
    PRIVATE piece
    PRIVATE random
)

target_link_libraries(perft PRIVATE "cul")

set_target_properties(perft PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
