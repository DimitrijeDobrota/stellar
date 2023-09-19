add_library(bit INTERFACE)
target_include_directories(bit INTERFACE .)
stellar_target_precompile_headers(bit INTERFACE "bit.hpp")

add_library(bitboard INTERFACE bitboard.cpp)
target_include_directories(bitboard INTERFACE .)
stellar_target_precompile_headers(bitboard INTERFACE "bitboard.hpp")

add_library(color INTERFACE)
target_include_directories(color INTERFACE .)
stellar_target_precompile_headers(color INTERFACE "color.hpp")

add_library(square INTERFACE)
target_include_directories(square INTERFACE .)
stellar_target_precompile_headers(square INTERFACE "square.hpp")

add_library(timer INTERFACE)
target_include_directories(timer INTERFACE .)
stellar_target_precompile_headers(timer INTERFACE "timer.hpp")

add_library(utils INTERFACE)
target_include_directories(utils INTERFACE .)
stellar_target_precompile_headers(utils INTERFACE "utils.hpp")
