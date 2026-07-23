#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/core/square.hpp"
#include "chess/search/search.hpp"

namespace {

void test_find_best_move_returns_legal_move_in_start_position() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from != chess::NoSquare);
    assert(result.best_move.to != chess::NoSquare);
}

void test_find_best_move_captures_high_value_piece() {
    chess::Board board = chess::board_from_fen("4k3/8/8/4q3/4R3/8/8/4K3 w - - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from == 28);
    assert(result.best_move.to == 36);
    assert(result.score == 500);
}

void test_find_best_move_returns_no_move_when_no_legal_moves_exist() {
    chess::Board board = chess::board_from_fen("7k/5Q2/7K/8/8/8/8/8 b - - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from == chess::NoSquare);
    assert(result.best_move.to == chess::NoSquare);
}

}

int main() {
    test_find_best_move_returns_legal_move_in_start_position();
    test_find_best_move_captures_high_value_piece();
    test_find_best_move_returns_no_move_when_no_legal_moves_exist();

    return 0;
}
