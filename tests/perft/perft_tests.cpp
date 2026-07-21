#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/movegen/perft.hpp"

namespace {

void test_start_position_perft() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert(chess::perft(board, 0) == 1);
    assert(chess::perft(board, 1) == 20);
    assert(chess::perft(board, 2) == 400);
    assert(chess::perft(board, 3) == 8902);
    assert(chess::perft(board, 4) == 197281);
    assert(chess::perft(board, 5) == 4865609);
}

void test_position_with_castling_rights_perft() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    assert(chess::perft(board, 1) == 26);
}

void test_position_with_en_passant_perft() {
    chess::Board board = chess::board_from_fen("8/8/8/3pP3/8/8/8/4K2k w - d6 0 1");
    assert(chess::perft(board, 1) == 7);
}

void test_position_with_promotion_perft() {
    chess::Board board = chess::board_from_fen("7k/P7/8/8/8/8/8/4K3 w - - 0 1");
    assert(chess::perft(board, 1) == 9);
}

}

int main() {
    test_start_position_perft();
    test_position_with_castling_rights_perft();
    test_position_with_en_passant_perft();
    test_position_with_promotion_perft();

    return 0;
}
