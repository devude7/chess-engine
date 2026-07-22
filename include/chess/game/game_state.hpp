#pragma once

#include "chess/board/board.hpp"

namespace chess {

enum class GameResult {
    Ongoing,
    WhiteWon,
    BlackWon,
    Draw
};

bool is_checkmate(const Board& board);
bool is_stalemate(const Board& board);
GameResult game_result(const Board& board);

}
