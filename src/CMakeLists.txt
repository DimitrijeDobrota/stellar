file(GLOB files "*")
foreach(file in ${files})
    if(IS_DIRECTORY "${file}" AND EXISTS "${file}/CMakeLists.txt")
        get_filename_component(mod ${file} NAME)
        message(STATUS "Found target ${mod}")
        add_subdirectory(${mod})
    endif()
endforeach()


configure_file(stellar_version.cpp.in stellar_version.cpp @ONLY)
add_library(Stellar_version STATIC ${CMAKE_CURRENT_BINARY_DIR}/stellar_version.cpp)
target_include_directories(Stellar_version PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
