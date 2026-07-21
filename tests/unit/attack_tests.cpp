#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/core/square.hpp"
#include "chess/movegen/attack.hpp"

void test_pawn_attacks() {
    chess::Board board = chess::board_from_fen("8/8/8/8/4P3/8/8/8 w - - 0 1");

    assert(chess::is_square_attacked(board, 35, chess::Color::White));
    assert(chess::is_square_attacked(board, 37, chess::Color::White));
    assert(!chess::is_square_attacked(board, 36, chess::Color::White));
}

void test_knight_attacks() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3N4/8/8/8 w - - 0 1");

    assert(chess::is_square_attacked(board, 10, chess::Color::White));
    assert(chess::is_square_attacked(board, 44, chess::Color::White));
    assert(!chess::is_square_attacked(board, 28, chess::Color::White));
}

void test_sliding_attacks_stop_on_blocker() {
    chess::Board board = chess::board_from_fen("8/8/3p4/3P4/3R4/8/8/8 w - - 0 1");

    assert(chess::is_square_attacked(board, 35, chess::Color::White));
    assert(!chess::is_square_attacked(board, 43, chess::Color::White));
}

void test_king_attacks() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3K4/8/8/8 w - - 0 1");

    assert(chess::is_square_attacked(board, 35, chess::Color::White));
    assert(!chess::is_square_attacked(board, 43, chess::Color::White));
}

void test_is_in_check() {
    chess::Board board = chess::board_from_fen("4r3/8/8/8/8/8/8/4K3 w - - 0 1");

    assert(chess::king_square(board, chess::Color::White) == 4);
    assert(chess::is_in_check(board, chess::Color::White));
    assert(!chess::is_in_check(board, chess::Color::Black));
}

int main() {
    test_pawn_attacks();
    test_knight_attacks();
    test_sliding_attacks_stop_on_blocker();
    test_king_attacks();
    test_is_in_check();

    return 0;
}
