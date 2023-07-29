cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    Stellar
	VERSION 0.0.3
    DESCRIPTION "Chess engine written in C"
	HOMEPAGE_URL https://git.dimitrijedobrota.com/stellar.git
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory(src)
