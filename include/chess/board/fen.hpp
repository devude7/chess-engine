#pragma once

#include <string>

#include "chess/board/board.hpp"

namespace chess {

Board board_from_fen(const std::string& fen);

}
