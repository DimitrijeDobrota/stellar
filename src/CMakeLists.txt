file(GLOB files "*")
foreach(file in ${files})
    if(IS_DIRECTORY "${file}" AND EXISTS "${file}/CMakeLists.txt")
        get_filename_component(mod ${file} NAME)
        message(STATUS "Found target ${mod}")
        add_subdirectory(${mod})
    endif()
endforeach()

