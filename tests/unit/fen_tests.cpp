#include <cassert>

#include "chess/board/fen.hpp"
#include "chess/core/piece.hpp"
#include "chess/core/square.hpp"

void test_single_piece() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/R7 w - - 0 1");

    chess::Piece piece = chess::piece_at(board, 0);

    assert(piece.type == chess::PieceType::Rook);
    assert(piece.color == chess::Color::White);
    assert(board.side_to_move == chess::Color::White);
    assert(board.en_passant_square == chess::NoSquare);
    assert(board.halfmove_clock == 0);
    assert(board.fullmove_number == 1);
}

void test_start_position() {
    chess::Board board = chess::board_from_fen(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    chess::Piece white_rook = chess::piece_at(board, 0);
    chess::Piece white_king = chess::piece_at(board, 4);
    chess::Piece white_pawn = chess::piece_at(board, 8);
    chess::Piece black_pawn = chess::piece_at(board, 48);
    chess::Piece black_rook = chess::piece_at(board, 56);
    chess::Piece black_king = chess::piece_at(board, 60);

    assert(white_rook.type == chess::PieceType::Rook);
    assert(white_rook.color == chess::Color::White);
    assert(white_king.type == chess::PieceType::King);
    assert(white_king.color == chess::Color::White);
    assert(white_pawn.type == chess::PieceType::Pawn);
    assert(white_pawn.color == chess::Color::White);

    assert(black_pawn.type == chess::PieceType::Pawn);
    assert(black_pawn.color == chess::Color::Black);
    assert(black_rook.type == chess::PieceType::Rook);
    assert(black_rook.color == chess::Color::Black);
    assert(black_king.type == chess::PieceType::King);
    assert(black_king.color == chess::Color::Black);

    assert(board.side_to_move == chess::Color::White);
    assert(board.castling_rights.white_kingside);
    assert(board.castling_rights.white_queenside);
    assert(board.castling_rights.black_kingside);
    assert(board.castling_rights.black_queenside);
    assert(board.en_passant_square == chess::NoSquare);
    assert(board.halfmove_clock == 0);
    assert(board.fullmove_number == 1);
}

void test_black_to_move_en_passant_and_counters() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/8/8 b - e3 12 34");

    assert(board.side_to_move == chess::Color::Black);
    assert(!board.castling_rights.white_kingside);
    assert(!board.castling_rights.white_queenside);
    assert(!board.castling_rights.black_kingside);
    assert(!board.castling_rights.black_queenside);
    assert(board.en_passant_square == 20);
    assert(board.halfmove_clock == 12);
    assert(board.fullmove_number == 34);
}

int main() {
    test_single_piece();
    test_start_position();
    test_black_to_move_en_passant_and_counters();

    return 0;
}
