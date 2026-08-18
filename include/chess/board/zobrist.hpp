#pragma once

#include <cstdint>

#include "chess/board/board.hpp"

namespace chess {

std::uint64_t zobrist_hash(const Board& board);

}
