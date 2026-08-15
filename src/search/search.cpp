#include "chess/search/search.hpp"

#include <algorithm>
#include <limits>
#include <vector>

#include "chess/core/square.hpp"
#include "chess/movegen/attack.hpp"
#include "chess/movegen/move_generator.hpp"
#include "chess/search/evaluation.hpp"

namespace chess {

namespace {

constexpr int Infinity = 1000000;
constexpr int MateScore = 100000;

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

int negamax(Board& board, int depth, int alpha, int beta) {
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
    order_moves(board, moves);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha);
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
    int alpha = -Infinity;
    int beta = Infinity;
    order_moves(board, moves);

    for (Move move : moves) {
        UndoState undo{};

        if (!make_move(board, move, undo)) {
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha);
        undo_move(board, move, undo);

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return SearchResult{best_move, best_score};
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

}
