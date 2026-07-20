#include <cassert>

#include "chess/board/board.hpp"
#include "chess/board/fen.hpp"

void test_make_and_undo_normal_move() {
    chess::Board board = chess::board_from_fen("8/8/8/8/8/8/4P3/8 w - - 0 1");
    chess::UndoState undo{};

    bool ok = chess::make_move(board, chess::Move{12, 28, chess::MoveType::Normal, chess::PieceType::None}, undo);

    assert(ok);
    assert(chess::is_empty_square(board, 12));
    assert(chess::piece_at(board, 28).type == chess::PieceType::Pawn);
    assert(board.side_to_move == chess::Color::Black);
    assert(board.en_passant_square == 20);

    chess::undo_move(board, chess::Move{12, 28, chess::MoveType::Normal, chess::PieceType::None}, undo);

    assert(chess::piece_at(board, 12).type == chess::PieceType::Pawn);
    assert(chess::is_empty_square(board, 28));
    assert(board.side_to_move == chess::Color::White);
    assert(board.en_passant_square == chess::NoSquare);
}

void test_make_and_undo_capture() {
    chess::Board board = chess::board_from_fen("8/8/8/3p4/4P3/8/8/8 w - - 0 1");
    chess::UndoState undo{};
    chess::Move move{28, 35, chess::MoveType::Normal, chess::PieceType::None};

    bool ok = chess::make_move(board, move, undo);

    assert(ok);
    assert(chess::piece_at(board, 35).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 35).color == chess::Color::White);
    assert(chess::is_empty_square(board, 28));

    chess::undo_move(board, move, undo);

    assert(chess::piece_at(board, 28).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 28).color == chess::Color::White);
    assert(chess::piece_at(board, 35).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 35).color == chess::Color::Black);
}

void test_make_and_undo_promotion() {
    chess::Board board = chess::board_from_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    chess::UndoState undo{};
    chess::Move move{48, 56, chess::MoveType::Promotion, chess::PieceType::Queen};

    bool ok = chess::make_move(board, move, undo);

    assert(ok);
    assert(chess::piece_at(board, 56).type == chess::PieceType::Queen);

    chess::undo_move(board, move, undo);

    assert(chess::piece_at(board, 48).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 48).color == chess::Color::White);
    assert(chess::is_empty_square(board, 56));
}

void test_make_and_undo_en_passant() {
    chess::Board board = chess::board_from_fen("8/8/8/3pP3/8/8/8/8 w - d6 0 1");
    chess::UndoState undo{};
    chess::Move move{36, 43, chess::MoveType::EnPassant, chess::PieceType::None};

    bool ok = chess::make_move(board, move, undo);

    assert(ok);
    assert(chess::piece_at(board, 43).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 43).color == chess::Color::White);
    assert(chess::is_empty_square(board, 36));
    assert(chess::is_empty_square(board, 35));

    chess::undo_move(board, move, undo);

    assert(chess::piece_at(board, 36).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 36).color == chess::Color::White);
    assert(chess::piece_at(board, 35).type == chess::PieceType::Pawn);
    assert(chess::piece_at(board, 35).color == chess::Color::Black);
    assert(chess::is_empty_square(board, 43));
    assert(board.en_passant_square == 43);
}

int main() {
    test_make_and_undo_normal_move();
    test_make_and_undo_capture();
    test_make_and_undo_promotion();
    test_make_and_undo_en_passant();

    return 0;
}
