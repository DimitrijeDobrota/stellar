add_executable(perft perft.cpp)

option(STELLAR_FULL_COUNT "Make count on types of moves" OFF)
if(STELLAR_FULL_COUNT)
    add_definitions(-DUSE_FULL_COUNT)
endif()

target_link_libraries(perft
    PRIVATE board
    PRIVATE moves
    PRIVATE piece
    PRIVATE utils
)

target_precompile_headers(perft
    REUSE_FROM engine
)

set_target_properties(perft PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# position and results taken from chessprogramming.com

set(STELLAR_PERFT_START_TEST "Start" "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
set(STELLAR_PERFT_START_RESULTS 20 400 8902 197281 4865609 119060324 3195901860)

set(STELLAR_PERFT_TRICKY_TEST "Tricky" "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1")
set(STELLAR_PERFT_TRICKY_RESULTS 48 2039 97862 4085603 193690690 8031647685)

set(STELLAR_PERFT_ENDGAME_TEST "Endgame" "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -")
set(STELLAR_PERFT_ENDGAME_RESULTS 14 191 2812 43238 674624 11030083 178633661 3009794393)

set(STELLAR_PERFT_MIDGAME_TEST "Midgame" "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1")
set(STELLAR_PERFT_MIDGAME_RESULTS 6 264 9467 422333 15833292 706045033)

set(STELLAR_PERFT_RMIDGAME_TEST "Reverse Midgame" "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1")
set(STELLAR_PERFT_RMIDGAME_RESULTS 6 264 9467 422333 15833292 706045033)

set(STELLAR_PERFT_BUGGY_TEST "Buggy" "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8")
set(STELLAR_PERFT_BUGGY_RESULTS 44 1486 62379 2103487 89941194)

set(STELLAR_PERFT_TEST START MIDGAME RMIDGAME ENDGAME TRICKY BUGGY)

foreach(test_name ${STELLAR_PERFT_TEST})
    set(test STELLAR_PERFT_${test_name}_TEST)
    set(res STELLAR_PERFT_${test_name}_RESULTS)
    list(GET ${test} 0 name)
    list(GET ${test} 1 fen)
    list(LENGTH ${res} size)
    foreach(depth RANGE 1 ${size})
        math(EXPR index "${depth} - 1")
        list(GET ${res} ${index} test_expected)
        set(test_name "${name} Position Depth ${depth}")
        add_test(NAME ${test_name} COMMAND perft -f "${fen}" -t8 -d${depth})
        set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION ${test_expected})
    endforeach()
endforeach()
