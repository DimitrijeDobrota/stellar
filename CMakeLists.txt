cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    Stellar
	VERSION 1.3.0
    DESCRIPTION "Chess engine written in C++"
	HOMEPAGE_URL https://git.dimitrijedobrota.com/stellar.git
    LANGUAGES CXX
)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING
    "Enable profileing for the project"
    FORCE
)

mark_as_advanced(CMAKE_CXX_FLAGS_PROFILE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Profile"
    FORCE
)

add_subdirectory(src)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

if( lto_supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()
