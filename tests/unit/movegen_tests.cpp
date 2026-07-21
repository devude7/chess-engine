#include <cassert>
#include <vector>

#include "chess/board/fen.hpp"
#include "chess/core/move.hpp"
#include "chess/movegen/move_generator.hpp"

namespace {

bool contains_move(const std::vector<chess::Move>& moves, int from, int to) {
    for (const chess::Move& move : moves) {
        if (move.from == from && move.to == to) {
            return true;
        }
    }

    return false;
}

bool contains_move(
    const std::vector<chess::Move>& moves,
    int from,
    int to,
    chess::MoveType type,
    chess::PieceType promotion) {
    for (const chess::Move& move : moves) {
        if (move.from == from && move.to == to && move.type == type && move.promotion == promotion) {
            return true;
        }
    }

    return false;
}

void test_knight_in_center() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3N4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 8);
    assert(contains_move(moves, 27, 10));
    assert(contains_move(moves, 27, 12));
    assert(contains_move(moves, 27, 17));
    assert(contains_move(moves, 27, 21));
    assert(contains_move(moves, 27, 33));
    assert(contains_move(moves, 27, 37));
    assert(contains_move(moves, 27, 42));
    assert(contains_move(moves, 27, 44));
}

void test_knight_in_corner() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/N7 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 2);
    assert(contains_move(moves, 0, 10));
    assert(contains_move(moves, 0, 17));
}

void test_king_in_center() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3K4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 8);
    assert(contains_move(moves, 27, 18));
    assert(contains_move(moves, 27, 19));
    assert(contains_move(moves, 27, 20));
    assert(contains_move(moves, 27, 26));
    assert(contains_move(moves, 27, 28));
    assert(contains_move(moves, 27, 34));
    assert(contains_move(moves, 27, 35));
    assert(contains_move(moves, 27, 36));
}

void test_king_in_corner() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/K7 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 3);
    assert(contains_move(moves, 0, 1));
    assert(contains_move(moves, 0, 8));
    assert(contains_move(moves, 0, 9));
}

void test_bishop_in_center() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3B4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 13);
    assert(contains_move(moves, 27, 0));
    assert(contains_move(moves, 27, 6));
    assert(contains_move(moves, 27, 48));
    assert(contains_move(moves, 27, 63));
}

void test_rook_in_center() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3R4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 14);
    assert(contains_move(moves, 27, 3));
    assert(contains_move(moves, 27, 24));
    assert(contains_move(moves, 27, 31));
    assert(contains_move(moves, 27, 59));
}

void test_queen_in_center() {
    chess::Board board = chess::board_from_fen("8/8/8/8/3Q4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 27);
    assert(contains_move(moves, 27, 0));
    assert(contains_move(moves, 27, 3));
    assert(contains_move(moves, 27, 31));
    assert(contains_move(moves, 27, 63));
}

void test_sliding_piece_stops_before_own_piece() {
    chess::Board board = chess::board_from_fen("8/8/3P4/8/3R4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(contains_move(moves, 27, 35));
    assert(!contains_move(moves, 27, 43));
    assert(!contains_move(moves, 27, 51));
}

void test_sliding_piece_captures_enemy_and_stops() {
    chess::Board board = chess::board_from_fen("8/8/3p4/8/3R4/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(contains_move(moves, 27, 35));
    assert(contains_move(moves, 27, 43));
    assert(!contains_move(moves, 27, 51));
}

void test_white_pawn_initial_double_move() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/4P3/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 2);
    assert(contains_move(moves, 12, 20));
    assert(contains_move(moves, 12, 28));
}

void test_black_pawn_initial_double_move() {
    chess::Board board = chess::board_from_fen("8/4p3/8/8/8/8/8/8 b - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 2);
    assert(contains_move(moves, 52, 44));
    assert(contains_move(moves, 52, 36));
}

void test_pawn_captures() {
    chess::Board board = chess::board_from_fen("8/8/8/3p1p2/4P3/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 3);
    assert(contains_move(moves, 28, 35));
    assert(contains_move(moves, 28, 36));
    assert(contains_move(moves, 28, 37));
}

void test_pawn_promotion() {
    chess::Board board = chess::board_from_fen("8/P7/8/8/8/8/8/8 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 4);
    assert(contains_move(moves, 48, 56, chess::MoveType::Promotion, chess::PieceType::Queen));
    assert(contains_move(moves, 48, 56, chess::MoveType::Promotion, chess::PieceType::Rook));
    assert(contains_move(moves, 48, 56, chess::MoveType::Promotion, chess::PieceType::Bishop));
    assert(contains_move(moves, 48, 56, chess::MoveType::Promotion, chess::PieceType::Knight));
}

void test_en_passant() {
    chess::Board board = chess::board_from_fen("8/8/8/3pP3/8/8/8/8 w - d6 0 1");

    std::vector<chess::Move> moves = chess::generate_pseudo_legal_moves(board);

    assert(moves.size() == 2);
    assert(contains_move(moves, 36, 44));
    assert(contains_move(moves, 36, 43, chess::MoveType::EnPassant, chess::PieceType::None));
}

} 

int main() {
    test_knight_in_center();
    test_knight_in_corner();
    test_king_in_center();
    test_king_in_corner();
    test_bishop_in_center();
    test_rook_in_center();
    test_queen_in_center();
    test_sliding_piece_stops_before_own_piece();
    test_sliding_piece_captures_enemy_and_stops();
    test_white_pawn_initial_double_move();
    test_black_pawn_initial_double_move();
    test_pawn_captures();
    test_pawn_promotion();
    test_en_passant();

    return 0;
}
