#pragma once

#include "chess/board/board.hpp"
#include "chess/core/move.hpp"
#include "chess/core/move_list.hpp"

namespace chess {

MoveList generate_pseudo_legal_moves(const Board& board);
MoveList generate_pseudo_tactical_moves(const Board& board);
MoveList generate_legal_moves(const Board& board);
MoveList generate_tactical_moves(const Board& board);

}
