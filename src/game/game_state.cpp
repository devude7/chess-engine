#include "chess/game/game_state.hpp"

#include <vector>

#include "chess/core/square.hpp"
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

bool is_fifty_move_rule_draw(const Board& board) {
    return board.halfmove_clock >= 100;
}

bool has_insufficient_material(const Board& board) {
    int bishop_count = 0;
    int knight_count = 0;
    int bishop_square_color = -1;

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (is_empty(piece) || piece.type == PieceType::King) {
            continue;
        }

        if (piece.type == PieceType::Pawn || piece.type == PieceType::Rook || piece.type == PieceType::Queen) {
            return false;
        }

        if (piece.type == PieceType::Knight) {
            ++knight_count;
            continue;
        }

        if (piece.type == PieceType::Bishop) {
            ++bishop_count;
            int current_bishop_square_color = (file_of(square) + rank_of(square)) % 2;

            if (bishop_square_color == -1) {
                bishop_square_color = current_bishop_square_color;
            } else if (bishop_square_color != current_bishop_square_color) {
                return false;
            }
        }
    }

    if (bishop_count == 0 && knight_count == 0) {
        return true;
    }

    if (bishop_count + knight_count == 1) {
        return true;
    }

    return bishop_count > 0 && knight_count == 0 && bishop_square_color != -1;
}

bool is_draw_by_rules(const Board& board) {
    return is_stalemate(board)
        || is_fifty_move_rule_draw(board)
        || has_insufficient_material(board);
}

GameResult game_result(const Board& board) {

    if (is_checkmate(board) && board.side_to_move == Color::White) {
        return GameResult::BlackWon;
    } 
    else if (is_checkmate(board) && board.side_to_move == Color::Black) {
        return GameResult::WhiteWon;
    } 
    else if (is_draw_by_rules(board)) {
        return GameResult::Draw;
    }
    
    return GameResult::Ongoing;
}

}
