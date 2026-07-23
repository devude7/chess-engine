#pragma once

#include "chess/board/board.hpp"
#include "chess/core/move.hpp"

namespace chess {

struct SearchResult {
    Move best_move;
    int score;
};

SearchResult find_best_move(Board& board, int depth);

}
