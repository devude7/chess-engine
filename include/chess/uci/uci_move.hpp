#pragma once

#include <string>

#include "chess/board/board.hpp"
#include "chess/core/move.hpp"

namespace chess {

std::string move_to_uci(Move move);
Move move_from_uci(const Board& board, const std::string& text);

}
