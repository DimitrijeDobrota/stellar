add_library(attack INTERFACE)
target_link_libraries(attack INTERFACE utils)
target_include_directories(attack INTERFACE .)
stellar_target_precompile_headers(attack INTERFACE "attack.hpp")
