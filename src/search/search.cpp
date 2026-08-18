#include "chess/search/search.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "chess/board/position_key.hpp"
#include "chess/core/square.hpp"
#include "chess/movegen/attack.hpp"
#include "chess/movegen/move_generator.hpp"
#include "chess/search/evaluation.hpp"

namespace chess {

namespace {

constexpr int Infinity = 1000000;
constexpr int MateScore = 100000;
constexpr int RepetitionPenalty = 200;

enum class BoundType {
    Exact,
    LowerBound,
    UpperBound
};

struct TranspositionEntry {
    int depth;
    int score;
    BoundType bound;
};

using TranspositionTable = std::unordered_map<std::string, TranspositionEntry>;

struct SearchContext {
    SearchStats stats;
    TranspositionTable table;
};

Move no_move() {
    return Move{NoSquare, NoSquare, MoveType::Normal, PieceType::None};
}

int evaluate_for_side_to_move(const Board& board) {
    int score = evaluate(board);

    return board.side_to_move == Color::White ? score : -score;
}

int captured_square_for_move(Move move, Piece moving_piece) {
    if (move.type == MoveType::EnPassant) {
        return moving_piece.color == Color::White ? move.to - 8 : move.to + 8;
    }

    return move.to;
}

void order_moves(const Board& board, std::vector<Move>& moves) {
    std::sort(moves.begin(), moves.end(), [&board](Move left, Move right) {
        return move_order_score(board, left) > move_order_score(board, right);
    });
}

bool contains_position(const std::vector<std::string>& positions, const std::string& key) {
    return std::find(positions.begin(), positions.end(), key) != positions.end();
}

int negamax(Board& board, int depth, int alpha, int beta, SearchContext& context) {
    ++context.stats.nodes;

    if (depth == 0) {
        return quiescence(board, alpha, beta, context.stats);
    }

    int original_alpha = alpha;
    int original_beta = beta;
    std::string key = position_key(board);
    auto found = context.table.find(key);

    if (found != context.table.end() && found->second.depth >= depth) {
        const TranspositionEntry& entry = found->second;

        if (entry.bound == BoundType::Exact) {
            ++context.stats.tt_hits;
            return entry.score;
        }

        if (entry.bound == BoundType::LowerBound && entry.score >= beta) {
            ++context.stats.tt_hits;
            return entry.score;
        }

        if (entry.bound == BoundType::UpperBound && entry.score <= alpha) {
            ++context.stats.tt_hits;
            return entry.score;
        }
    }

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty()) {
        if (is_in_check(board, board.side_to_move)) {
            return -MateScore - depth;
        }

        return 0;
    }

    int best_score = std::numeric_limits<int>::min();
    order_moves(board, moves);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha, context);
        undo_move(board, move, undo);

        if (score > best_score) {
            best_score = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }

    BoundType bound = BoundType::Exact;

    if (best_score <= original_alpha) {
        bound = BoundType::UpperBound;
    } else if (best_score >= original_beta) {
        bound = BoundType::LowerBound;
    }

    context.table[key] = TranspositionEntry{depth, best_score, bound};

    return best_score;
}

}

SearchResult find_best_move(Board& board, int depth) {
    return find_best_move(board, depth, std::vector<std::string>{});
}

SearchResult find_best_move(Board& board, int depth, const std::vector<std::string>& recent_positions) {
    std::vector<Move> moves = generate_legal_moves(board);
    SearchContext context{SearchStats{1, 0}, TranspositionTable{}};

    if (moves.empty() || depth <= 0) {
        return SearchResult{no_move(), evaluate_for_side_to_move(board), context.stats};
    }

    Move best_move = moves.front();
    int best_score = std::numeric_limits<int>::min();
    int alpha = -Infinity;
    int beta = Infinity;
    order_moves(board, moves);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha, context);

        if (contains_position(recent_positions, position_key(board))) {
            score -= RepetitionPenalty;
        }

        undo_move(board, move, undo);

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return SearchResult{best_move, best_score, context.stats};
}

int move_order_score(const Board& board, Move move) {
    int score = 0;
    Piece moving_piece = piece_at(board, move.from);

    if (move.type == MoveType::Promotion) {
        score += 80000 + piece_value(move.promotion);
    }

    int captured_square = captured_square_for_move(move, moving_piece);

    if (is_valid_square(captured_square)) {
        Piece captured_piece = piece_at(board, captured_square);

        if (!is_empty(captured_piece)) {
            score += 10000 + piece_value(captured_piece.type) * 10 - piece_value(moving_piece.type);
        }
    }

    return score;
}

int quiescence(Board& board, int alpha, int beta) {
    SearchStats stats{0, 0};
    return quiescence(board, alpha, beta, stats);
}

int quiescence(Board& board, int alpha, int beta, SearchStats& stats) {
    ++stats.nodes;

    int stand_pat = evaluate_for_side_to_move(board);

    if (stand_pat >= beta) {
        return beta;
    }

    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    std::vector<Move> moves = generate_tactical_moves(board);
    order_moves(board, moves);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -quiescence(board, -beta, -alpha, stats);
        undo_move(board, move, undo);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

}
