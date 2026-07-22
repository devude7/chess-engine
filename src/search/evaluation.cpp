#include "chess/search/evaluation.hpp"

namespace chess {

int piece_value(PieceType piece_type) {
    switch (piece_type) {
        case PieceType::Pawn:
            return 100;
        case PieceType::Knight:
            return 320;
        case PieceType::Bishop:
            return 330;
        case PieceType::Rook:
            return 500;
        case PieceType::Queen:
            return 900;
        case PieceType::King:
        case PieceType::None:
            return 0;
    }
    return 0;
}

int evaluate(const Board& board) {
    int score = 0;

    for (Piece piece : board.squares) {
        int value = piece_value(piece.type);

        if (piece.color == Color::White) {
            score += value;
        } else {
            score -= value;
        }
    }
    return score;
}

}
