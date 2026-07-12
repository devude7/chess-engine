#pragma once 

#include "chess/core/color.hpp"

namespace chess {

enum class PieceType {
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct Piece {
    PieceType type;
    Color color;
};

bool is_empty(Piece piece);

}
