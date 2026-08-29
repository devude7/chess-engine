#include "chess/search/search.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

#include "chess/board/zobrist.hpp"
#include "chess/core/square.hpp"
#include "chess/game/game_state.hpp"
#include "chess/movegen/attack.hpp"
#include "chess/movegen/move_generator.hpp"
#include "chess/search/evaluation.hpp"

namespace chess {

namespace {

constexpr int Infinity = 1000000;
constexpr int MateScore = 100000;
constexpr int RepetitionPenalty = 200;
constexpr int MaxPly = 128;
constexpr int KillerMoveScore = 9000;
constexpr int HistoryScoreLimit = 1000000;
constexpr int InitialAspirationWindow = 50;
constexpr int MaxAspirationWindow = Infinity;
constexpr int NullMoveMinDepth = 3;
constexpr int NullMoveReduction = 2;

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
using KillerMoves = std::array<std::array<Move, 2>, MaxPly>;
using HistoryTable = std::array<std::array<int, 64>, 64>;

struct SearchContext {
    SearchStats stats;
    TranspositionTable table;
    KillerMoves killer_moves;
    HistoryTable history;
    std::vector<std::uint64_t> position_history;
    std::function<bool()> should_stop;
    bool stopped;
};

struct NullMoveState {
    Color side_to_move;
    int en_passant_square;
    int halfmove_clock;
    int fullmove_number;
    std::uint64_t position_hash;
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

KillerMoves empty_killer_moves() {
    KillerMoves killer_moves{};

    for (auto& ply_killers : killer_moves) {
        ply_killers[0] = no_move();
        ply_killers[1] = no_move();
    }

    return killer_moves;
}

HistoryTable empty_history_table() {
    HistoryTable history{};

    for (auto& row : history) {
        row.fill(0);
    }

    return history;
}

SearchContext make_search_context(
    const std::vector<std::uint64_t>& position_history = std::vector<std::uint64_t>{},
    const std::function<bool()>& should_stop = std::function<bool()>{}
) {
    return SearchContext{
        SearchStats{0, 0},
        TranspositionTable{},
        empty_killer_moves(),
        empty_history_table(),
        position_history,
        should_stop,
        false
    };
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

bool is_capture(const Board& board, Move move) {
    Piece moving_piece = piece_at(board, move.from);
    int captured_square = captured_square_for_move(move, moving_piece);

    return is_valid_square(captured_square) && !is_empty(piece_at(board, captured_square));
}

bool is_quiet_move(const Board& board, Move move) {
    return move.type != MoveType::Promotion
        && move.type != MoveType::EnPassant
        && !is_capture(board, move);
}

bool is_killer_move(const KillerMoves& killer_moves, int ply, Move move) {
    if (ply < 0 || ply >= MaxPly) {
        return false;
    }

    return same_move(killer_moves[ply][0], move) || same_move(killer_moves[ply][1], move);
}

void store_killer_move(KillerMoves& killer_moves, int ply, Move move) {
    if (ply < 0 || ply >= MaxPly || same_move(killer_moves[ply][0], move)) {
        return;
    }

    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = move;
}

void add_history_score(HistoryTable& history, Move move, int depth) {
    if (!is_valid_square(move.from) || !is_valid_square(move.to)) {
        return;
    }

    history[move.from][move.to] += depth * depth;

    if (history[move.from][move.to] > HistoryScoreLimit) {
        history[move.from][move.to] = HistoryScoreLimit;
    }
}

int history_score(const HistoryTable& history, Move move) {
    if (!is_valid_square(move.from) || !is_valid_square(move.to)) {
        return 0;
    }

    return history[move.from][move.to];
}

int ordered_move_score(
    const Board& board,
    Move move,
    Move tt_move,
    bool has_tt_move,
    const KillerMoves& killer_moves,
    const HistoryTable& history,
    int ply
) {
    if (has_tt_move && same_move(move, tt_move)) {
        return 1000000;
    }

    int score = move_order_score(board, move);

    if (is_quiet_move(board, move) && is_killer_move(killer_moves, ply, move)) {
        score += KillerMoveScore;
    }

    if (is_quiet_move(board, move)) {
        score += history_score(history, move);
    }

    return score;
}

void order_moves(
    const Board& board,
    std::vector<Move>& moves,
    Move tt_move,
    bool has_tt_move,
    const KillerMoves& killer_moves,
    const HistoryTable& history,
    int ply
) {
    std::sort(moves.begin(), moves.end(), [&board, tt_move, has_tt_move, &killer_moves, &history, ply](Move left, Move right) {
        return ordered_move_score(board, left, tt_move, has_tt_move, killer_moves, history, ply)
            > ordered_move_score(board, right, tt_move, has_tt_move, killer_moves, history, ply);
    });
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

bool contains_position(const std::vector<std::uint64_t>& positions, std::uint64_t key) {
    return std::find(positions.begin(), positions.end(), key) != positions.end();
}

int count_position(const std::vector<std::uint64_t>& positions, std::uint64_t key) {
    return static_cast<int>(std::count(positions.begin(), positions.end(), key));
}

bool is_repetition_draw(const SearchContext& context, const Board& board) {
    return count_position(context.position_history, board.position_hash) >= 3;
}

bool is_search_draw(const SearchContext& context, const Board& board) {
    return is_fifty_move_rule_draw(board)
        || has_insufficient_material(board)
        || is_repetition_draw(context, board);
}

bool has_non_pawn_material_for_color(const Board& board, Color color) {
    for (Piece piece : board.squares) {
        if (piece.color == color && piece.type != PieceType::None && piece.type != PieceType::Pawn && piece.type != PieceType::King) {
            return true;
        }
    }

    return false;
}

bool can_try_null_move(const Board& board, int depth) {
    return depth >= NullMoveMinDepth
        && !is_in_check(board, board.side_to_move)
        && has_non_pawn_material_for_color(board, board.side_to_move);
}

NullMoveState make_null_move(Board& board) {
    NullMoveState state{
        board.side_to_move,
        board.en_passant_square,
        board.halfmove_clock,
        board.fullmove_number,
        board.position_hash
    };

    if (board.en_passant_square != NoSquare) {
        board.position_hash ^= zobrist_en_passant_key(board.en_passant_square);
    }

    board.side_to_move = opposite(board.side_to_move);
    board.en_passant_square = NoSquare;
    board.position_hash ^= zobrist_side_to_move_key();
    ++board.halfmove_clock;

    if (state.side_to_move == Color::Black) {
        ++board.fullmove_number;
    }

    return state;
}

void undo_null_move(Board& board, const NullMoveState& state) {
    board.side_to_move = state.side_to_move;
    board.en_passant_square = state.en_passant_square;
    board.halfmove_clock = state.halfmove_clock;
    board.fullmove_number = state.fullmove_number;
    board.position_hash = state.position_hash;
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
        Move move = tt_best_move(table, current_board.position_hash);

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

int quiescence_search(Board& board, int ply, int alpha, int beta, SearchStats& stats) {
    ++stats.nodes;

    if (is_in_check(board, board.side_to_move)) {
        std::vector<Move> moves = generate_legal_moves(board);

        if (moves.empty()) {
            return -MateScore + ply;
        }

        order_moves(board, moves);

        for (Move move : moves) {
            UndoState undo{};

            if (!make_move(board, move, undo)) {
                continue;
            }

            int score = -quiescence_search(board, ply + 1, -beta, -alpha, stats);
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

        int score = -quiescence_search(board, ply + 1, -beta, -alpha, stats);
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

int negamax(Board& board, int depth, int ply, int alpha, int beta, SearchContext& context) {
    ++context.stats.nodes;

    if (context.should_stop && context.should_stop()) {
        context.stopped = true;
        return evaluate_for_side_to_move(board);
    }

    if (depth == 0) {
        return quiescence_search(board, ply, alpha, beta, context.stats);
    }

    if (is_search_draw(context, board)) {
        return 0;
    }

    int original_alpha = alpha;
    int original_beta = beta;
    std::uint64_t key = board.position_hash;
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

    if (can_try_null_move(board, depth)) {
        NullMoveState null_move_state = make_null_move(board);
        int reduced_depth = depth - 1 - NullMoveReduction;

        if (reduced_depth < 0) {
            reduced_depth = 0;
        }

        int null_move_score = -negamax(board, reduced_depth, ply + 1, -beta, -beta + 1, context);
        undo_null_move(board, null_move_state);

        if (context.stopped) {
            return evaluate_for_side_to_move(board);
        }

        if (null_move_score >= beta) {
            context.table[key] = TranspositionEntry{depth, beta, BoundType::LowerBound, no_move(), false};
            return beta;
        }
    }

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty()) {
        if (is_in_check(board, board.side_to_move)) {
            return -MateScore + ply;
        }

        return 0;
    }

    int best_score = std::numeric_limits<int>::min();
    Move best_move = no_move();
    order_moves(board, moves, tt_move, has_tt_move, context.killer_moves, context.history, ply);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        std::uint64_t child_key = board.position_hash;
        context.position_history.push_back(child_key);
        int score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, context);
        context.position_history.pop_back();
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
            if (is_quiet_move(board, move)) {
                store_killer_move(context.killer_moves, ply, move);
                add_history_score(context.history, move, depth);
            }

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
    const std::vector<std::uint64_t>& recent_positions,
    SearchContext& context,
    int alpha,
    int beta
) {
    ++context.stats.nodes;

    std::vector<Move> moves = generate_legal_moves(board);

    if (moves.empty() || depth <= 0) {
        return SearchResult{no_move(), {}, evaluate_for_side_to_move(board), context.stats};
    }

    if (is_search_draw(context, board)) {
        return SearchResult{moves.front(), {}, 0, context.stats};
    }

    std::uint64_t key = board.position_hash;
    Move root_tt_move = tt_best_move(context.table, key);
    bool has_root_tt_move = root_tt_move.from != NoSquare;
    Move best_move = moves.front();
    int best_score = std::numeric_limits<int>::min();
    int original_alpha = alpha;
    int original_beta = beta;
    order_moves(board, moves, root_tt_move, has_root_tt_move);
    bool searched_move = false;

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        std::uint64_t child_key = board.position_hash;
        context.position_history.push_back(child_key);
        int score = -negamax(board, depth - 1, 1, -beta, -alpha, context);
        context.position_history.pop_back();

        if (contains_position(recent_positions, child_key)) {
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

        if (alpha >= beta) {
            break;
        }
    }

    if (!searched_move) {
        return SearchResult{best_move, {}, evaluate_for_side_to_move(board), context.stats};
    }

    if (!context.stopped) {
        BoundType bound = BoundType::Exact;

        if (best_score <= original_alpha) {
            bound = BoundType::UpperBound;
        } else if (best_score >= original_beta) {
            bound = BoundType::LowerBound;
        }

        context.table[key] = TranspositionEntry{depth, best_score, bound, best_move, best_move.from != NoSquare};
    }

    std::vector<Move> principal_variation = principal_variation_from_table(board, context.table, depth);
    return SearchResult{best_move, principal_variation, best_score, context.stats};
}

}

SearchResult find_best_move(Board& board, int depth) {
    return find_best_move(board, depth, std::vector<std::uint64_t>{});
}

SearchResult find_best_move(Board& board, int depth, const std::vector<std::uint64_t>& recent_positions) {
    SearchContext context = make_search_context(recent_positions);
    return find_best_move_with_context(board, depth, recent_positions, context, -Infinity, Infinity);
}

SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::uint64_t>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished
) {
    return find_best_move_iterative(board, max_depth, recent_positions, on_depth_finished, std::function<bool()>{});
}

SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::uint64_t>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished,
    const std::function<bool()>& should_stop
) {
    SearchContext context = make_search_context(recent_positions, should_stop);
    SearchResult result{no_move(), {}, evaluate_for_side_to_move(board), context.stats};
    bool completed_depth = false;
    int previous_score = 0;

    for (int depth = 1; depth <= max_depth; ++depth) {
        int window = InitialAspirationWindow;
        int alpha = completed_depth ? previous_score - window : -Infinity;
        int beta = completed_depth ? previous_score + window : Infinity;
        SearchResult depth_result{no_move(), {}, evaluate_for_side_to_move(board), context.stats};

        while (true) {
            context.stopped = false;
            depth_result = find_best_move_with_context(board, depth, recent_positions, context, alpha, beta);

            if (context.stopped) {
                if (!completed_depth) {
                    result = depth_result;
                }

                break;
            }

            if (!completed_depth || (depth_result.score > alpha && depth_result.score < beta)) {
                break;
            }

            if (depth_result.score <= alpha) {
                alpha = previous_score - window * 2;
            } else {
                beta = previous_score + window * 2;
            }

            if (alpha <= -Infinity && beta >= Infinity) {
                break;
            }

            if (alpha < -Infinity) {
                alpha = -Infinity;
            }

            if (beta > Infinity) {
                beta = Infinity;
            }

            if (window < MaxAspirationWindow / 2) {
                window *= 2;
            } else {
                window = MaxAspirationWindow;
            }
        }

        if (context.stopped) {
            break;
        }

        result = depth_result;
        previous_score = result.score;
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
    return quiescence_search(board, 0, alpha, beta, stats);
}

}
