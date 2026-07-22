#include "chess/game/game_state.hpp"

#include <vector>

#include "chess/core/move.hpp"
#include "chess/movegen/attack.hpp"
#include "chess/movegen/move_generator.hpp"

namespace chess {

bool is_checkmate(const Board& board) {
    std::vector<Move> legal_moves = generate_legal_moves(board);

    return is_in_check(board, board.side_to_move) && legal_moves.empty();
}

bool is_stalemate(const Board& board) {
    std::vector<Move> legal_moves = generate_legal_moves(board);

    return !is_in_check(board, board.side_to_move) && legal_moves.empty();
}

GameResult game_result(const Board& board) {

    if (is_checkmate(board) && board.side_to_move == Color::White) {
        return GameResult::BlackWon;
    } 
    else if (is_checkmate(board) && board.side_to_move == Color::Black) {
        return GameResult::WhiteWon;
    } 
    else if (is_stalemate(board)) {
        return GameResult::Draw;
    }
    
    return GameResult::Ongoing;
}

}
