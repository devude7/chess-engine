#include <cassert>
#include <string>

#include "chess/board/fen.hpp"
#include "chess/core/square.hpp"
#include "chess/uci/uci_move.hpp"

namespace {

void test_normal_move_to_uci() {
    chess::Move move{12, 28, chess::MoveType::Normal, chess::PieceType::None};

    assert(chess::move_to_uci(move) == "e2e4");
}

void test_promotion_move_to_uci() {
    chess::Move move{52, 60, chess::MoveType::Promotion, chess::PieceType::Queen};

    assert(chess::move_to_uci(move) == "e7e8q");
}

void test_castling_move_to_uci() {
    chess::Move move{4, 6, chess::MoveType::Castling, chess::PieceType::None};

    assert(chess::move_to_uci(move) == "e1g1");
}

void test_invalid_move_to_uci() {
    chess::Move move{chess::NoSquare, chess::NoSquare, chess::MoveType::Normal, chess::PieceType::None};

    assert(chess::move_to_uci(move) == "0000");
}

void test_normal_move_from_uci() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    chess::Move move = chess::move_from_uci(board, "e2e4");

    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.type == chess::MoveType::Normal);
}

void test_promotion_move_from_uci() {
    chess::Board board = chess::board_from_fen("7k/4P3/8/8/8/8/8/4K3 w - - 0 1");

    chess::Move move = chess::move_from_uci(board, "e7e8q");

    assert(move.from == 52);
    assert(move.to == 60);
    assert(move.type == chess::MoveType::Promotion);
    assert(move.promotion == chess::PieceType::Queen);
}

void test_castling_move_from_uci() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    chess::Move move = chess::move_from_uci(board, "e1g1");

    assert(move.from == 4);
    assert(move.to == 6);
    assert(move.type == chess::MoveType::Castling);
}

void test_illegal_move_from_uci_returns_no_move() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");

    chess::Move move = chess::move_from_uci(board, "e1e8");

    assert(move.from == chess::NoSquare);
    assert(move.to == chess::NoSquare);
}

}

int main() {
    test_normal_move_to_uci();
    test_promotion_move_to_uci();
    test_castling_move_to_uci();
    test_invalid_move_to_uci();
    test_normal_move_from_uci();
    test_promotion_move_from_uci();
    test_castling_move_from_uci();
    test_illegal_move_from_uci_returns_no_move();

    return 0;
}
