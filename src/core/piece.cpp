#include "chess/core/piece.hpp"

namespace chess {

bool is_empty(Piece piece) {
    return piece.type == PieceType::None;
}

}
