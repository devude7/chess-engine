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
    src/movegen/move_generator.cpp
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
