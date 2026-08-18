#include <cassert>
#include <vector>

#include "chess/board/fen.hpp"
#include "chess/board/position_key.hpp"
#include "chess/core/square.hpp"
#include "chess/search/evaluation.hpp"
#include "chess/search/search.hpp"

namespace {

void test_find_best_move_returns_legal_move_in_start_position() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from != chess::NoSquare);
    assert(result.best_move.to != chess::NoSquare);
    assert(result.stats.nodes > 0);
    assert(result.stats.tt_hits == 0);
}

void test_find_best_move_captures_high_value_piece() {
    chess::Board board = chess::board_from_fen("4k3/8/8/4q3/4R3/8/8/4K3 w - - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from == 28);
    assert(result.best_move.to == 36);
    assert(result.score > 0);
}

void test_find_best_move_returns_no_move_when_no_legal_moves_exist() {
    chess::Board board = chess::board_from_fen("7k/5Q2/7K/8/8/8/8/8 b - - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 1);

    assert(result.best_move.from == chess::NoSquare);
    assert(result.best_move.to == chess::NoSquare);
}

void test_capture_scores_higher_than_quiet_move() {
    chess::Board board = chess::board_from_fen("4k3/8/8/4q3/4R3/8/8/4K3 w - - 0 1");

    chess::Move capture{28, 36, chess::MoveType::Normal, chess::PieceType::None};
    chess::Move quiet{28, 29, chess::MoveType::Normal, chess::PieceType::None};

    assert(chess::move_order_score(board, capture) > chess::move_order_score(board, quiet));
}

void test_promotion_scores_higher_than_quiet_move() {
    chess::Board board = chess::board_from_fen("7k/4P3/8/8/8/8/8/4K3 w - - 0 1");

    chess::Move promotion{52, 60, chess::MoveType::Promotion, chess::PieceType::Queen};
    chess::Move quiet{4, 5, chess::MoveType::Normal, chess::PieceType::None};

    assert(chess::move_order_score(board, promotion) > chess::move_order_score(board, quiet));
}

void test_quiescence_returns_static_evaluation_in_quiet_position() {
    chess::Board board = chess::board_from_fen("7k/8/8/8/8/8/8/4K3 w - - 0 1");

    assert(chess::quiescence(board, -1000000, 1000000) == chess::evaluate(board));
}

void test_quiescence_sees_immediate_capture() {
    chess::Board board = chess::board_from_fen("4r1k1/8/8/4R3/8/8/8/4K3 b - - 0 1");

    int static_score_for_black = -chess::evaluate(board);
    int quiescence_score = chess::quiescence(board, -1000000, 1000000);

    assert(quiescence_score > static_score_for_black);
    assert(quiescence_score > 0);
}

void test_repetition_penalty_can_change_root_choice() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/R3K2k w Q - 0 1");
    chess::SearchResult without_history = chess::find_best_move(board, 1);

    chess::Board after_best_move = board;
    chess::UndoState undo{};
    chess::make_move(after_best_move, without_history.best_move, undo);

    std::vector<std::string> recent_positions{chess::position_key(after_best_move)};
    chess::SearchResult with_history = chess::find_best_move(board, 1, recent_positions);

    bool changed_move = with_history.best_move.from != without_history.best_move.from
        || with_history.best_move.to != without_history.best_move.to;
    bool penalized_same_move = with_history.score < without_history.score;

    assert(without_history.best_move.from != chess::NoSquare);
    assert(with_history.best_move.from != chess::NoSquare);
    assert(changed_move || penalized_same_move);
}

void test_transposition_table_is_used_during_deeper_search() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/R3K2k w Q - 0 1");

    chess::SearchResult result = chess::find_best_move(board, 4);

    assert(result.stats.tt_hits > 0);
}

void test_iterative_search_reports_each_completed_depth() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/R3K2k w Q - 0 1");
    std::vector<int> completed_depths;

    chess::SearchResult result = chess::find_best_move_iterative(
        board,
        3,
        std::vector<std::string>{},
        [&completed_depths](int depth, const chess::SearchResult&) {
            completed_depths.push_back(depth);
        }
    );

    assert(result.best_move.from != chess::NoSquare);
    assert(completed_depths.size() == 3);
    assert(completed_depths[0] == 1);
    assert(completed_depths[1] == 2);
    assert(completed_depths[2] == 3);
}

}

int main() {
    test_find_best_move_returns_legal_move_in_start_position();
    test_find_best_move_captures_high_value_piece();
    test_find_best_move_returns_no_move_when_no_legal_moves_exist();
    test_capture_scores_higher_than_quiet_move();
    test_promotion_scores_higher_than_quiet_move();
    test_quiescence_returns_static_evaluation_in_quiet_position();
    test_quiescence_sees_immediate_capture();
    test_repetition_penalty_can_change_root_choice();
    test_transposition_table_is_used_during_deeper_search();
    test_iterative_search_reports_each_completed_depth();

    return 0;
}
