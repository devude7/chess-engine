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
bool is_fifty_move_rule_draw(const Board& board);
bool has_insufficient_material(const Board& board);
bool is_draw_by_rules(const Board& board);
GameResult game_result(const Board& board);

}
