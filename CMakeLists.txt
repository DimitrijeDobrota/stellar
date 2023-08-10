cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    Stellar
	VERSION 0.0.19
    DESCRIPTION "Chess engine written in C"
	HOMEPAGE_URL https://git.dimitrijedobrota.com/stellar.git
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-ops-limit=1000000000")

add_subdirectory(src)
