#pragma once

#include "chess/core/piece.hpp"

namespace chess {

enum class MoveType {
    Normal,
    Promotion,
    EnPassant,
    Castling
};

struct Move {
    int from;
    int to;
    MoveType type;
    PieceType promotion;
};

bool is_promotion(Move move);

}
