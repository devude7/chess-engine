#pragma once

#include "chess/board/board.hpp"
#include "chess/core/piece.hpp"

namespace chess {

int piece_value(PieceType piece_type);
int piece_square_value(Piece piece, int square);
int evaluate(const Board& board);

}
