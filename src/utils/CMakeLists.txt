add_library(bit INTERFACE)
target_include_directories(bit INTERFACE .)

add_library(bitboard INTERFACE bitboard.cpp)
target_include_directories(bitboard INTERFACE .)

add_library(color INTERFACE)
target_include_directories(color INTERFACE .)

add_library(square INTERFACE)
target_include_directories(square INTERFACE .)

add_library(timer INTERFACE)
target_include_directories(timer INTERFACE .)

add_library(utils INTERFACE)
target_include_directories(utils INTERFACE .)
