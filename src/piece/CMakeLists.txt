add_library(piece INTERFACE)

target_link_libraries(piece
    INTERFACE attack
    INTERFACE utils
    INTERFACE color
)

target_include_directories(piece INTERFACE .)
stellar_target_precompile_headers(piece INTERFACE "piece.hpp")

