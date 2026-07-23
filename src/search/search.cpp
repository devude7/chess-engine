#include "chess/search/search.hpp"

#include <limits>
#include <vector>

#include "chess/core/square.hpp"
#include "chess/movegen/attack.hpp"
#include "chess/movegen/move_generator.hpp"
#include "chess/search/evaluation.hpp"

namespace chess {

namespace {

constexpr int MateScore = 100000;

Move no_move() {
    return Move{NoSquare, NoSquare, MoveType::Normal, PieceType::None};
}

int evaluate_for_side_to_move(const Board& board) {
    int score = evaluate(board);

    return board.side_to_move == Color::White ? score : -score;
}

int negamax(Board& board, int depth) {
    if (depth == 0) {
        return evaluate_for_side_to_move(board);
    }

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty()) {
        if (is_in_check(board, board.side_to_move)) {
            return -MateScore - depth;
        }

        return 0;
    }

    int best_score = std::numeric_limits<int>::min();

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1);
        undo_move(board, move, undo);

        if (score > best_score) {
            best_score = score;
        }
    }

    return best_score;
}

}

SearchResult find_best_move(Board& board, int depth) {
    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty() || depth <= 0) {
        return SearchResult{no_move(), evaluate_for_side_to_move(board)};
    }

    Move best_move = moves.front();
    int best_score = std::numeric_limits<int>::min();

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1);
        undo_move(board, move, undo);

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
    }

    return SearchResult{best_move, best_score};
}

}
