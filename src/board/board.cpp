#include "chess/board/board.hpp"

namespace chess {

Piece piece_at(const Board& board, int square) {
    return board.squares[square];
}

void set_piece(Board& board, int square, Piece piece) {
    board.squares[square] = piece;
}

Board empty_board() {
    Board board{};

    for (auto& piece : board.squares) {
        piece = Piece{PieceType::None, Color::White};
    }

    board.side_to_move = Color::White;
    board.castling_rights = CastlingRights{false, false, false, false};
    board.en_passant_square = NoSquare;
    board.halfmove_clock = 0;
    board.fullmove_number = 1;

    return board;
}

bool is_empty_square(const Board& board, int square) {
    return is_empty(piece_at(board, square));
}

bool is_occupied_by(const Board& board, int square, Color color) {
    Piece piece = piece_at(board, square);
    return !is_empty(piece) && piece.color == color;
}

} 
