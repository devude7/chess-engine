#include "chess/search/search.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "chess/board/position_key.hpp"
#include "chess/board/zobrist.hpp"
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
    Move best_move;
    bool has_best_move;
};

using TranspositionTable = std::unordered_map<std::uint64_t, TranspositionEntry>;

struct SearchContext {
    SearchStats stats;
    TranspositionTable table;
    std::function<bool()> should_stop;
    bool stopped;
};

Move no_move() {
    return Move{NoSquare, NoSquare, MoveType::Normal, PieceType::None};
}

bool same_move(Move left, Move right) {
    return left.from == right.from
        && left.to == right.to
        && left.type == right.type
        && left.promotion == right.promotion;
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

void order_moves(const Board& board, std::vector<Move>& moves, Move tt_move, bool has_tt_move) {
    std::sort(moves.begin(), moves.end(), [&board, tt_move, has_tt_move](Move left, Move right) {
        if (has_tt_move) {
            bool left_is_tt_move = same_move(left, tt_move);
            bool right_is_tt_move = same_move(right, tt_move);

            if (left_is_tt_move != right_is_tt_move) {
                return left_is_tt_move;
            }
        }

        return move_order_score(board, left) > move_order_score(board, right);
    });
}

void order_moves(const Board& board, std::vector<Move>& moves) {
    order_moves(board, moves, no_move(), false);
}

bool contains_position(const std::vector<std::string>& positions, const std::string& key) {
    return std::find(positions.begin(), positions.end(), key) != positions.end();
}

Move tt_best_move(const TranspositionTable& table, std::uint64_t key) {
    auto found = table.find(key);

    if (found == table.end() || !found->second.has_best_move) {
        return no_move();
    }

    return found->second.best_move;
}

std::vector<Move> principal_variation_from_table(const Board& board, const TranspositionTable& table, int depth) {
    std::vector<Move> line;
    Board current_board = board;

    for (int ply = 0; ply < depth; ++ply) {
        Move move = tt_best_move(table, zobrist_hash(current_board));

        if (move.from == NoSquare || move.to == NoSquare) {
            break;
        }

        UndoState undo{};

        if (!make_move(current_board, move, undo)) {
            break;
        }

        line.push_back(move);
    }

    return line;
}

int negamax(Board& board, int depth, int alpha, int beta, SearchContext& context) {
    ++context.stats.nodes;

    if (context.should_stop && context.should_stop()) {
        context.stopped = true;
        return evaluate_for_side_to_move(board);
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta, context.stats);
    }

    int original_alpha = alpha;
    int original_beta = beta;
    std::uint64_t key = zobrist_hash(board);
    auto found = context.table.find(key);
    Move tt_move = no_move();
    bool has_tt_move = false;

    if (found != context.table.end()) {
        const TranspositionEntry& entry = found->second;
        tt_move = entry.best_move;
        has_tt_move = entry.has_best_move;

        if (entry.depth >= depth) {
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
    }

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty()) {
        if (is_in_check(board, board.side_to_move)) {
            return -MateScore - depth;
        }

        return 0;
    }

    int best_score = std::numeric_limits<int>::min();
    Move best_move = no_move();
    order_moves(board, moves, tt_move, has_tt_move);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha, context);
        undo_move(board, move, undo);

        if (context.stopped) {
            return evaluate_for_side_to_move(board);
        }

        if (score > best_score) {
            best_score = score;
            best_move = move;
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

    context.table[key] = TranspositionEntry{depth, best_score, bound, best_move, best_move.from != NoSquare};

    return best_score;
}

SearchResult find_best_move_with_context(
    Board& board,
    int depth,
    const std::vector<std::string>& recent_positions,
    SearchContext& context
) {
    ++context.stats.nodes;

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty() || depth <= 0) {
        return SearchResult{no_move(), {}, evaluate_for_side_to_move(board), context.stats};
    }

    std::uint64_t key = zobrist_hash(board);
    Move root_tt_move = tt_best_move(context.table, key);
    bool has_root_tt_move = root_tt_move.from != NoSquare;
    Move best_move = moves.front();
    int best_score = std::numeric_limits<int>::min();
    int alpha = -Infinity;
    int beta = Infinity;
    order_moves(board, moves, root_tt_move, has_root_tt_move);
    bool searched_move = false;

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

        if (context.stopped) {
            break;
        }

        searched_move = true;

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    if (!searched_move) {
        return SearchResult{best_move, {}, evaluate_for_side_to_move(board), context.stats};
    }

    if (!context.stopped) {
        context.table[key] = TranspositionEntry{depth, best_score, BoundType::Exact, best_move, best_move.from != NoSquare};
    }

    std::vector<Move> principal_variation = principal_variation_from_table(board, context.table, depth);
    return SearchResult{best_move, principal_variation, best_score, context.stats};
}

}

SearchResult find_best_move(Board& board, int depth) {
    return find_best_move(board, depth, std::vector<std::string>{});
}

SearchResult find_best_move(Board& board, int depth, const std::vector<std::string>& recent_positions) {
    SearchContext context{SearchStats{0, 0}, TranspositionTable{}, std::function<bool()>{}, false};
    return find_best_move_with_context(board, depth, recent_positions, context);
}

SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::string>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished
) {
    return find_best_move_iterative(board, max_depth, recent_positions, on_depth_finished, std::function<bool()>{});
}

SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::string>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished,
    const std::function<bool()>& should_stop
) {
    SearchContext context{SearchStats{0, 0}, TranspositionTable{}, should_stop, false};
    SearchResult result{no_move(), {}, evaluate_for_side_to_move(board), context.stats};
    bool completed_depth = false;

    for (int depth = 1; depth <= max_depth; ++depth) {
        context.stopped = false;
        SearchResult depth_result = find_best_move_with_context(board, depth, recent_positions, context);

        if (context.stopped) {
            if (!completed_depth) {
                result = depth_result;
            }

            break;
        }

        result = depth_result;
        completed_depth = true;

        if (on_depth_finished) {
            on_depth_finished(depth, result);
        }

        if (context.should_stop && context.should_stop()) {
            break;
        }
    }

    return result;
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
