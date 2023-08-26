add_library(random INTERFACE)
target_link_libraries(piece INTERFACE utils)
target_include_directories(random INTERFACE .)
stellar_target_precompile_headers(random INTERFACE "random.hpp")
