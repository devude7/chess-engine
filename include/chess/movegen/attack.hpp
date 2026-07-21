#pragma once

#include "chess/board/board.hpp"
#include "chess/core/color.hpp"

namespace chess {

bool is_square_attacked(const Board& board, int square, Color by_color);
int king_square(const Board& board, Color color);
bool is_in_check(const Board& board, Color color);

}
