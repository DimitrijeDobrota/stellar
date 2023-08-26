cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    Stellar
	VERSION 0.0.21
    DESCRIPTION "Chess engine written in C"
	HOMEPAGE_URL https://git.dimitrijedobrota.com/stellar.git
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)

option(STELLAR_ENABLE_PCH "Build using pre-compiled headers" ON)
function(stellar_target_precompile_headers)
    if(STELLAR_ENABLE_PCH)
        target_precompile_headers(${ARGV})
    endif()
endfunction()

CHECK_CXX_COMPILER_FLAG("-flto -fno-fat-lto-objects" COMPILER_SUPPORTS_LTO)
if(COMPILER_SUPPORTS_LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fno-fat-lto-objects")
endif()

CHECK_CXX_COMPILER_FLAG("-fconstexpr-ops-limit=10000000000" COMPILER_SUPPORTS_CONSTEXPR_OPS_LIMIT)
if(COMPILER_SUPPORTS_CONSTEXPR_OPS_LIMIT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-ops-limit=1000000000")
endif()

CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

add_subdirectory(src)
