add_library(random INTERFACE)
target_link_libraries(piece INTERFACE utils)
target_include_directories(random INTERFACE .)
