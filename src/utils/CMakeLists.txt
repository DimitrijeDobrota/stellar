add_library(utils utils.cpp)
target_include_directories(utils INTERFACE .)
stellar_target_precompile_headers(attack INTERFACE "utils.hpp")
