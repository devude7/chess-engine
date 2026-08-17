#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/search/evaluation.hpp"

namespace {

void test_piece_values() {
    assert(chess::piece_value(chess::PieceType::None) == 0);
    assert(chess::piece_value(chess::PieceType::Pawn) == 100);
    assert(chess::piece_value(chess::PieceType::Knight) == 320);
    assert(chess::piece_value(chess::PieceType::Bishop) == 330);
    assert(chess::piece_value(chess::PieceType::Rook) == 500);
    assert(chess::piece_value(chess::PieceType::Queen) == 900);
    assert(chess::piece_value(chess::PieceType::King) == 0);
}

void test_start_position_is_equal() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    assert(chess::evaluate(board) == 0);
}

void test_white_material_advantage_is_positive() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/Q6k w - - 0 1");

    assert(chess::evaluate(board) > 0);
}

void test_black_material_advantage_is_negative() {
    chess::Board board = chess::board_from_fen("r6k/8/8/8/8/8/8/8 b - - 0 1");

    assert(chess::evaluate(board) < 0);
}

void test_knight_is_better_in_center_than_corner() {
    chess::Piece white_knight{chess::PieceType::Knight, chess::Color::White};

    assert(chess::piece_square_value(white_knight, 27) > chess::piece_square_value(white_knight, 0));
}

void test_black_piece_square_table_is_mirrored() {
    chess::Piece white_pawn{chess::PieceType::Pawn, chess::Color::White};
    chess::Piece black_pawn{chess::PieceType::Pawn, chess::Color::Black};

    assert(chess::piece_square_value(white_pawn, 12) == chess::piece_square_value(black_pawn, 52));
}

void test_developed_knight_improves_evaluation() {
    chess::Board undeveloped = chess::board_from_fen("8/8/8/8/8/8/8/N6k w - - 0 1");
    chess::Board developed = chess::board_from_fen("8/8/8/8/3N4/8/8/7k w - - 0 1");

    assert(chess::evaluate(developed) > chess::evaluate(undeveloped));
}

void test_winning_endgame_prefers_losing_king_on_edge() {
    chess::Board king_in_center = chess::board_from_fen("8/8/8/3k4/8/8/8/Q3K3 w - - 0 1");
    chess::Board king_on_edge = chess::board_from_fen("7k/8/8/8/8/8/8/Q3K3 w - - 0 1");

    assert(chess::evaluate(king_on_edge) > chess::evaluate(king_in_center));
}

}

int main() {
    test_piece_values();
    test_start_position_is_equal();
    test_white_material_advantage_is_positive();
    test_black_material_advantage_is_negative();
    test_knight_is_better_in_center_than_corner();
    test_black_piece_square_table_is_mirrored();
    test_developed_knight_improves_evaluation();
    test_winning_endgame_prefers_losing_king_on_edge();

    return 0;
}
