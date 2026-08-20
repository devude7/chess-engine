#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/game/game_state.hpp"

namespace {

void test_checkmate_is_detected() {
    chess::Board board = chess::board_from_fen("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");

    assert(chess::is_checkmate(board));
    assert(!chess::is_stalemate(board));
    assert(chess::game_result(board) == chess::GameResult::BlackWon);
}

void test_stalemate_is_detected() {
    chess::Board board = chess::board_from_fen("7k/5Q2/7K/8/8/8/8/8 b - - 0 1");

    assert(chess::is_stalemate(board));
    assert(!chess::is_checkmate(board));
    assert(chess::game_result(board) == chess::GameResult::Draw);
}

void test_normal_position_is_not_finished() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    assert(!chess::is_checkmate(board));
    assert(!chess::is_stalemate(board));
    assert(chess::game_result(board) == chess::GameResult::Ongoing);
}

void test_fifty_move_rule_draw_is_detected() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 100 51");

    assert(chess::is_fifty_move_rule_draw(board));
    assert(chess::is_draw_by_rules(board));
    assert(chess::game_result(board) == chess::GameResult::Draw);
}

void test_insufficient_material_draw_is_detected() {
    chess::Board king_vs_king = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board king_and_bishop_vs_king = chess::board_from_fen("8/8/8/8/8/8/8/3BK2k w - - 0 1");
    chess::Board king_and_rook_vs_king = chess::board_from_fen("8/8/8/8/8/8/8/3RK2k w - - 0 1");

    assert(chess::has_insufficient_material(king_vs_king));
    assert(chess::has_insufficient_material(king_and_bishop_vs_king));
    assert(!chess::has_insufficient_material(king_and_rook_vs_king));
}

}

int main() {
    test_checkmate_is_detected();
    test_stalemate_is_detected();
    test_normal_position_is_not_finished();
    test_fifty_move_rule_draw_is_detected();
    test_insufficient_material_draw_is_detected();

    return 0;
}
