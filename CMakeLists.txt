cmake_minimum_required(VERSION 3.16)

project(chess_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chess_engine_core
    src/core/color.cpp
    src/core/piece.cpp
    src/core/square.cpp
    src/core/move.cpp
    src/board/board.cpp
    src/board/fen.cpp
    src/game/game_state.cpp
    src/movegen/attack.cpp
    src/movegen/move_generator.cpp
    src/movegen/perft.cpp
    src/search/evaluation.cpp
    src/search/search.cpp
)

target_include_directories(chess_engine_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(fen_tests
    tests/unit/fen_tests.cpp
)

target_link_libraries(fen_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME fen_tests COMMAND fen_tests)

add_executable(movegen_tests
    tests/unit/movegen_tests.cpp
)

target_link_libraries(movegen_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME movegen_tests COMMAND movegen_tests)

add_executable(board_tests
    tests/unit/board_tests.cpp
)

target_link_libraries(board_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME board_tests COMMAND board_tests)

add_executable(attack_tests
    tests/unit/attack_tests.cpp
)

target_link_libraries(attack_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME attack_tests COMMAND attack_tests)

add_executable(game_state_tests
    tests/unit/game_state_tests.cpp
)

target_link_libraries(game_state_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME game_state_tests COMMAND game_state_tests)

add_executable(evaluation_tests
    tests/unit/evaluation_tests.cpp
)

target_link_libraries(evaluation_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME evaluation_tests COMMAND evaluation_tests)

add_executable(search_tests
    tests/unit/search_tests.cpp
)

target_link_libraries(search_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME search_tests COMMAND search_tests)

add_executable(perft_tests
    tests/perft/perft_tests.cpp
)

target_link_libraries(perft_tests
    PRIVATE
        chess_engine_core
)

add_test(NAME perft_tests COMMAND perft_tests)
