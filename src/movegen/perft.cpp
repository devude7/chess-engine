#include "chess/movegen/perft.hpp"

#include "chess/core/move.hpp"
#include "chess/movegen/move_generator.hpp"

namespace chess {

std::uint64_t perft(Board& board, int depth) {
    if (depth == 0) {
        return 1;
    }

    MoveList moves = generate_legal_moves(board);

    if (depth == 1) {
        return moves.size();
    }

    std::uint64_t nodes = 0;

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        nodes += perft(board, depth - 1);
        undo_move(board, move, undo);
    }

    return nodes;
}

}
