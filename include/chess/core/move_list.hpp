#pragma once

#include <array>
#include <cassert>
#include <cstddef>

#include "chess/core/move.hpp"

namespace chess {

constexpr std::size_t MaxMoves = 256;

struct MoveList {
    std::array<Move, MaxMoves> moves{};
    std::size_t count = 0;

    void clear() {
        count = 0;
    }

    void push_back(Move move) {
        assert(count < MaxMoves);
        moves[count] = move;
        ++count;
    }

    bool empty() const {
        return count == 0;
    }

    std::size_t size() const {
        return count;
    }

    Move front() const {
        assert(!empty());
        return moves[0];
    }

    Move* begin() {
        return moves.data();
    }

    Move* end() {
        return moves.data() + count;
    }

    const Move* begin() const {
        return moves.data();
    }

    const Move* end() const {
        return moves.data() + count;
    }
};

}
