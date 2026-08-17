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

bool contains_move(const std::vector<chess::Move>& moves, int from, int to, chess::MoveType type, chess::PieceType promotion) {
    for (const chess::Move& move : moves) {
        if (move.from == from && move.to == to && move.type == type && move.promotion == promotion) {
            return true;
        }
    }

    return false;
}

bool same_move(chess::Move left, chess::Move right) {
    return left.from == right.from
        && left.to == right.to
        && left.type == right.type
        && left.promotion == right.promotion;
}

bool is_tactical_test_move(const chess::Board& board, chess::Move move) {
    return move.type == chess::MoveType::Promotion
        || move.type == chess::MoveType::EnPassant
        || !chess::is_empty(chess::piece_at(board, move.to));
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

void test_king_cannot_move_into_check() {
    chess::Board board = chess::board_from_fen("k3r3/8/8/8/8/8/8/4K3 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(!contains_move(moves, 4, 12));
    assert(contains_move(moves, 4, 3));
}

void test_pinned_piece_cannot_expose_king() {
    chess::Board board = chess::board_from_fen("k3r3/8/8/8/8/8/4R3/4K3 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(!contains_move(moves, 12, 11));
    assert(contains_move(moves, 12, 60));
}

void test_legal_moves_can_escape_check() {
    chess::Board board = chess::board_from_fen("k3r3/8/8/8/8/8/8/4K3 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(contains_move(moves, 4, 3));
    assert(contains_move(moves, 4, 5));
    assert(!contains_move(moves, 4, 12));
}

void test_castling_moves_are_generated() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(contains_move(moves, 4, 6, chess::MoveType::Castling, chess::PieceType::None));
    assert(contains_move(moves, 4, 2, chess::MoveType::Castling, chess::PieceType::None));
}

void test_castling_blocked_by_piece() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3KB1R w KQkq - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(!contains_move(moves, 4, 6, chess::MoveType::Castling, chess::PieceType::None));
    assert(contains_move(moves, 4, 2, chess::MoveType::Castling, chess::PieceType::None));
}

void test_castling_blocked_by_attacked_square() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/5r2/8/8/R3K2R w KQkq - 0 1");

    std::vector<chess::Move> moves = chess::generate_legal_moves(board);

    assert(!contains_move(moves, 4, 6, chess::MoveType::Castling, chess::PieceType::None));
    assert(contains_move(moves, 4, 2, chess::MoveType::Castling, chess::PieceType::None));
}

void test_tactical_moves_include_captures_only() {
    chess::Board board = chess::board_from_fen("4k3/8/8/4q3/4R3/8/8/4K3 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_tactical_moves(board);

    assert(contains_move(moves, 28, 36));
    assert(!contains_move(moves, 28, 29));
}

void test_tactical_moves_include_promotions() {
    chess::Board board = chess::board_from_fen("7k/4P3/8/8/8/8/8/4K3 w - - 0 1");

    std::vector<chess::Move> moves = chess::generate_tactical_moves(board);

    assert(moves.size() == 4);
    assert(contains_move(moves, 52, 60, chess::MoveType::Promotion, chess::PieceType::Queen));
}

void test_tactical_moves_include_en_passant() {
    chess::Board board = chess::board_from_fen("8/8/8/3pP3/8/8/8/4K2k w - d6 0 1");

    std::vector<chess::Move> moves = chess::generate_tactical_moves(board);

    assert(contains_move(moves, 36, 43, chess::MoveType::EnPassant, chess::PieceType::None));
    assert(!contains_move(moves, 36, 44));
}

void test_tactical_moves_match_filtered_legal_moves() {
    chess::Board board = chess::board_from_fen("r3k2r/1P3ppp/8/3pP3/3Nq3/8/PPP2PPP/R3K2R w KQkq d6 0 1");

    std::vector<chess::Move> legal_moves = chess::generate_legal_moves(board);
    std::vector<chess::Move> tactical_moves = chess::generate_tactical_moves(board);
    int expected_count = 0;

    for (chess::Move legal_move : legal_moves) {
        if (!is_tactical_test_move(board, legal_move)) {
            continue;
        }

        ++expected_count;
        bool found = false;

        for (chess::Move tactical_move : tactical_moves) {
            if (same_move(legal_move, tactical_move)) {
                found = true;
                break;
            }
        }

        assert(found);
    }

    assert(static_cast<int>(tactical_moves.size()) == expected_count);
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
    test_king_cannot_move_into_check();
    test_pinned_piece_cannot_expose_king();
    test_legal_moves_can_escape_check();
    test_castling_moves_are_generated();
    test_castling_blocked_by_piece();
    test_castling_blocked_by_attacked_square();
    test_tactical_moves_include_captures_only();
    test_tactical_moves_include_promotions();
    test_tactical_moves_include_en_passant();
    test_tactical_moves_match_filtered_legal_moves();

    return 0;
}
