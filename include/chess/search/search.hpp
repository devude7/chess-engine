#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "chess/board/board.hpp"
#include "chess/core/move.hpp"

namespace chess {

struct SearchStats {
    std::uint64_t nodes;
    std::uint64_t tt_hits;
};

struct SearchResult {
    Move best_move;
    std::vector<Move> principal_variation;
    int score;
    SearchStats stats;
};

SearchResult find_best_move(Board& board, int depth);
SearchResult find_best_move(Board& board, int depth, const std::vector<std::string>& recent_positions);
SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::string>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished
);
SearchResult find_best_move_iterative(
    Board& board,
    int max_depth,
    const std::vector<std::string>& recent_positions,
    const std::function<void(int, const SearchResult&)>& on_depth_finished,
    const std::function<bool()>& should_stop
);
int move_order_score(const Board& board, Move move);
int quiescence(Board& board, int alpha, int beta);
int quiescence(Board& board, int alpha, int beta, SearchStats& stats);

}
