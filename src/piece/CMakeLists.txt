add_library(piece INTERFACE)

target_link_libraries(piece
    INTERFACE attack
    INTERFACE utils
)

target_include_directories(piece INTERFACE .)
