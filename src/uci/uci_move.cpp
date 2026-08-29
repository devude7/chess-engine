#include "chess/uci/uci_move.hpp"

#include "chess/core/square.hpp"
#include "chess/movegen/move_generator.hpp"

namespace chess {

namespace {

Move no_move() {
    return Move{NoSquare, NoSquare, MoveType::Normal, PieceType::None};
}

char file_char(int square) {
    return static_cast<char>('a' + file_of(square));
}

char rank_char(int square) {
    return static_cast<char>('1' + rank_of(square));
}

char promotion_char(PieceType piece_type) {
    switch (piece_type) {
        case PieceType::Queen:
            return 'q';
        case PieceType::Rook:
            return 'r';
        case PieceType::Bishop:
            return 'b';
        case PieceType::Knight:
            return 'n';
        case PieceType::Pawn:
        case PieceType::King:
        case PieceType::None:
            return '\0';
    }

    return '\0';
}

}

std::string move_to_uci(Move move) {
    if (!is_valid_square(move.from) || !is_valid_square(move.to)) {
        return "0000";
    }

    std::string text;
    text += file_char(move.from);
    text += rank_char(move.from);
    text += file_char(move.to);
    text += rank_char(move.to);

    if (move.type == MoveType::Promotion) {
        char promotion = promotion_char(move.promotion);

        if (promotion != '\0') {
            text += promotion;
        }
    }

    return text;
}

Move move_from_uci(const Board& board, const std::string& text) {
    if (text.size() != 4 && text.size() != 5) {
        return no_move();
    }

    MoveList legal_moves = generate_legal_moves(board);

    for (Move move : legal_moves) {
        if (move_to_uci(move) == text) {
            return move;
        }
    }

    return no_move();
}

}
