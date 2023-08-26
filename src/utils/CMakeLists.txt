add_library(utils INTERFACE)
target_include_directories(utils INTERFACE .)
stellar_target_precompile_headers(attack INTERFACE "utils.hpp")
