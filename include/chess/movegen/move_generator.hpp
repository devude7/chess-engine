#pragma once

#include <vector>

#include "chess/board/board.hpp"
#include "chess/core/move.hpp"

namespace chess {

std::vector<Move> generate_pseudo_legal_moves(const Board& board);
std::vector<Move> generate_legal_moves(const Board& board);
std::vector<Move> generate_tactical_moves(const Board& board);

}
