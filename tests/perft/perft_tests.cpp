#include <cassert>
#include <cstdint>
#include <iostream>

#include "chess/board/fen.hpp"
#include "chess/movegen/perft.hpp"

namespace {

void assert_perft(chess::Board& board, int depth, std::uint64_t expected) {
    std::uint64_t actual = chess::perft(board, depth);

    if (actual != expected) {
        std::cerr << "perft(" << depth << ") expected " << expected << ", got " << actual << '\n';
    }

    assert(actual == expected);
}

void test_start_position_perft() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_perft(board, 0, 1);
    assert_perft(board, 1, 20);
    assert_perft(board, 2, 400);
    assert_perft(board, 3, 8902);
    assert_perft(board, 4, 197281);
    assert_perft(board, 5, 4865609);
}

void test_position_with_castling_rights_perft() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    assert_perft(board, 1, 26);
}

void test_position_with_en_passant_perft() {
    chess::Board board = chess::board_from_fen("8/8/8/3pP3/8/8/8/4K2k w - d6 0 1");
    assert_perft(board, 1, 7);
}

void test_position_with_promotion_perft() {
    chess::Board board = chess::board_from_fen("7k/P7/8/8/8/8/8/4K3 w - - 0 1");
    assert_perft(board, 1, 9);
}

void test_kiwipete_position_perft() {
    chess::Board board = chess::board_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    assert_perft(board, 1, 48);
    assert_perft(board, 2, 2039);
    assert_perft(board, 3, 97862);
}

void test_position_with_en_passant_edge_cases_perft() {
    chess::Board board = chess::board_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

    assert_perft(board, 1, 14);
    assert_perft(board, 2, 191);
    assert_perft(board, 3, 2812);
}

void test_position_with_promotions_and_checks_perft() {
    chess::Board board = chess::board_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

    assert_perft(board, 1, 6);
    assert_perft(board, 2, 264);
    assert_perft(board, 3, 9467);
}

}

int main() {
    test_start_position_perft();
    test_position_with_castling_rights_perft();
    test_position_with_en_passant_perft();
    test_position_with_promotion_perft();
    test_kiwipete_position_perft();
    test_position_with_en_passant_edge_cases_perft();
    test_position_with_promotions_and_checks_perft();

    return 0;
}
