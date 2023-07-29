cmake_minimum_required(VERSION 3.25.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    Stellar
	VERSION 0.0.0
    DESCRIPTION "Chess engine written in C"
	HOMEPAGE_URL https://git.dimitrijedobrota.com/stellar.git
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


file(GLOB sources "src/*.c")
add_executable(engine ${sources})

target_link_libraries(engine "cul")

set_target_properties(engine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(engine
    PRIVATE "${PROJECT_SOURCE_DIR}/include"
)
