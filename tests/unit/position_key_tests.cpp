#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/board/position_key.hpp"

namespace {

void test_position_key_ignores_move_counters() {
    chess::Board first = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board second = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 42 99");

    assert(chess::position_key(first) == chess::position_key(second));
}

void test_position_key_includes_side_to_move() {
    chess::Board white = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board black = chess::board_from_fen("8/8/8/8/8/8/8/4K2k b - - 0 1");

    assert(chess::position_key(white) != chess::position_key(black));
}

void test_position_key_includes_castling_and_en_passant() {
    chess::Board first = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    chess::Board second = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w - e3 0 1");

    assert(chess::position_key(first) != chess::position_key(second));
}

}

int main() {
    test_position_key_ignores_move_counters();
    test_position_key_includes_side_to_move();
    test_position_key_includes_castling_and_en_passant();

    return 0;
}
