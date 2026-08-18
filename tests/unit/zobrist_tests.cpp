#include <cassert>
#include <cstdint>

#include "chess/board/fen.hpp"
#include "chess/board/zobrist.hpp"

namespace {

void test_zobrist_hash_ignores_move_counters() {
    chess::Board first = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board second = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 42 99");

    assert(chess::zobrist_hash(first) == chess::zobrist_hash(second));
}

void test_zobrist_hash_includes_side_to_move() {
    chess::Board white = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board black = chess::board_from_fen("8/8/8/8/8/8/8/4K2k b - - 0 1");

    assert(chess::zobrist_hash(white) != chess::zobrist_hash(black));
}

void test_zobrist_hash_includes_castling_and_en_passant() {
    chess::Board first = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    chess::Board second = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w - e3 0 1");

    assert(chess::zobrist_hash(first) != chess::zobrist_hash(second));
}

void test_zobrist_hash_is_not_empty_for_position() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    assert(chess::zobrist_hash(board) != std::uint64_t{0});
}

}

int main() {
    test_zobrist_hash_ignores_move_counters();
    test_zobrist_hash_includes_side_to_move();
    test_zobrist_hash_includes_castling_and_en_passant();
    test_zobrist_hash_is_not_empty_for_position();

    return 0;
}
