#pragma once

#include <string>

#include "chess/board/board.hpp"

namespace chess {

bool load_nnue(const std::string& path);
void unload_nnue();

void set_nnue_enabled(bool enabled);
bool nnue_is_enabled();
bool nnue_is_loaded();

int evaluate_nnue(const Board& board);

}
