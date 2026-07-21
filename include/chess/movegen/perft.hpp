#pragma once

#include <cstdint>

#include "chess/board/board.hpp"

namespace chess {

std::uint64_t perft(Board& board, int depth);

}
