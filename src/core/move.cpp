#include "chess/core/move.hpp"

namespace chess {

bool is_promotion(Move move) {
    return move.type == MoveType::Promotion;
}

}
