#include "chess/board/board.hpp"

namespace chess {

namespace {

Piece empty_piece() {
    return Piece{PieceType::None, Color::White};
}

bool is_kingside_rook_square(int square, Color color) {
    return (color == Color::White && square == 7) || (color == Color::Black && square == 63);
}

bool is_queenside_rook_square(int square, Color color) {
    return (color == Color::White && square == 0) || (color == Color::Black && square == 56);
}

void remove_castling_rights_for_king(Board& board, Color color) {
    if (color == Color::White) {
        board.castling_rights.white_kingside = false;
        board.castling_rights.white_queenside = false;
    } else {
        board.castling_rights.black_kingside = false;
        board.castling_rights.black_queenside = false;
    }
}

void remove_castling_rights_for_rook(Board& board, int square, Color color) {
    if (is_kingside_rook_square(square, color)) {
        if (color == Color::White) {
            board.castling_rights.white_kingside = false;
        } else {
            board.castling_rights.black_kingside = false;
        }
    } else if (is_queenside_rook_square(square, color)) {
        if (color == Color::White) {
            board.castling_rights.white_queenside = false;
        } else {
            board.castling_rights.black_queenside = false;
        }
    }
}

void move_castling_rook(Board& board, Move move) {
    if (move.from == 4 && move.to == 6) {
        set_piece(board, 5, piece_at(board, 7));
        set_piece(board, 7, empty_piece());
    } else if (move.from == 4 && move.to == 2) {
        set_piece(board, 3, piece_at(board, 0));
        set_piece(board, 0, empty_piece());
    } else if (move.from == 60 && move.to == 62) {
        set_piece(board, 61, piece_at(board, 63));
        set_piece(board, 63, empty_piece());
    } else if (move.from == 60 && move.to == 58) {
        set_piece(board, 59, piece_at(board, 56));
        set_piece(board, 56, empty_piece());
    }
}

void undo_castling_rook(Board& board, Move move) {
    if (move.from == 4 && move.to == 6) {
        set_piece(board, 7, piece_at(board, 5));
        set_piece(board, 5, empty_piece());
    } else if (move.from == 4 && move.to == 2) {
        set_piece(board, 0, piece_at(board, 3));
        set_piece(board, 3, empty_piece());
    } else if (move.from == 60 && move.to == 62) {
        set_piece(board, 63, piece_at(board, 61));
        set_piece(board, 61, empty_piece());
    } else if (move.from == 60 && move.to == 58) {
        set_piece(board, 56, piece_at(board, 59));
        set_piece(board, 59, empty_piece());
    }
}

} 

Piece piece_at(const Board& board, int square) {
    return board.squares[square];
}

void set_piece(Board& board, int square, Piece piece) {
    board.squares[square] = piece;
}

Board empty_board() {
    Board board{};

    for (auto& piece : board.squares) {
        piece = empty_piece();
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

bool make_move(Board& board, Move move, UndoState& undo) {
    if (!is_valid_square(move.from) || !is_valid_square(move.to)) {
        return false;
    }

    Piece moving_piece = piece_at(board, move.from);
    if (is_empty(moving_piece) || moving_piece.color != board.side_to_move) {
        return false;
    }

    int captured_square = move.to;
    if (move.type == MoveType::EnPassant) {
        captured_square = moving_piece.color == Color::White ? move.to - 8 : move.to + 8;
    }

    Piece captured_piece = is_valid_square(captured_square) ? piece_at(board, captured_square) : empty_piece();

    undo.moved_piece = moving_piece;
    undo.captured_piece = captured_piece;
    undo.captured_square = captured_square;
    undo.castling_rights = board.castling_rights;
    undo.en_passant_square = board.en_passant_square;
    undo.halfmove_clock = board.halfmove_clock;
    undo.fullmove_number = board.fullmove_number;

    set_piece(board, move.from, empty_piece());

    if (move.type == MoveType::EnPassant) {
        set_piece(board, captured_square, empty_piece());
    }

    Piece placed_piece = moving_piece;
    if (move.type == MoveType::Promotion) {
        placed_piece.type = move.promotion;
    }

    set_piece(board, move.to, placed_piece);

    if (move.type == MoveType::Castling) {
        move_castling_rook(board, move);
    }

    if (moving_piece.type == PieceType::King) {
        remove_castling_rights_for_king(board, moving_piece.color);
    } else if (moving_piece.type == PieceType::Rook) {
        remove_castling_rights_for_rook(board, move.from, moving_piece.color);
    }

    if (!is_empty(captured_piece) && captured_piece.type == PieceType::Rook) {
        remove_castling_rights_for_rook(board, captured_square, captured_piece.color);
    }

    board.en_passant_square = NoSquare;
    if (moving_piece.type == PieceType::Pawn && (move.to - move.from == 16 || move.from - move.to == 16)) {
        board.en_passant_square = (move.from + move.to) / 2;
    }

    if (moving_piece.type == PieceType::Pawn || !is_empty(captured_piece)) {
        board.halfmove_clock = 0;
    } else {
        ++board.halfmove_clock;
    }

    if (moving_piece.color == Color::Black) {
        ++board.fullmove_number;
    }

    board.side_to_move = opposite(board.side_to_move);

    return true;
}

void undo_move(Board& board, Move move, const UndoState& undo) {
    if (move.type == MoveType::Castling) {
        undo_castling_rook(board, move);
    }

    set_piece(board, move.from, undo.moved_piece);
    set_piece(board, move.to, empty_piece());

    if (!is_empty(undo.captured_piece)) {
        set_piece(board, undo.captured_square, undo.captured_piece);
    }

    board.side_to_move = undo.moved_piece.color;
    board.castling_rights = undo.castling_rights;
    board.en_passant_square = undo.en_passant_square;
    board.halfmove_clock = undo.halfmove_clock;
    board.fullmove_number = undo.fullmove_number;
}

} 
