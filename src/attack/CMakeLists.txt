add_library(attack attack.cpp bishop.cpp rook.cpp)

target_link_libraries(attack
    PRIVATE bit
    PRIVATE bitboard
    PRIVATE color
    PRIVATE square
    PRIVATE utils
)

target_include_directories(attack INTERFACE .)
