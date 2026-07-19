#include "chess/movegen/move_generator.hpp"

namespace chess {

namespace {

void generate_pawn_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_knight_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_bishop_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_rook_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_queen_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_king_moves(const Board& board, int from, std::vector<Move>& moves);

} 

std::vector<Move> generate_pseudo_legal_moves(const Board& board) {
    std::vector<Move> moves;

    for (int square = 0; square < 64; ++square) {
        Piece piece = piece_at(board, square);

        if (piece.type == PieceType::None || board.side_to_move != piece.color) {
            continue;
        }

        if (piece.type == PieceType::Pawn) {
            generate_pawn_moves(board, square, moves);
        } else if (piece.type == PieceType::Knight) {
            generate_knight_moves(board, square, moves);
        } else if (piece.type == PieceType::Bishop) {
            generate_bishop_moves(board, square, moves);
        } else if (piece.type == PieceType::Rook) {
            generate_rook_moves(board, square, moves);
        } else if (piece.type == PieceType::Queen) {
            generate_queen_moves(board, square, moves);
        } else if (piece.type == PieceType::King) {
            generate_king_moves(board, square, moves);
        }
    }

    return moves;
}

namespace {

void generate_pawn_moves(const Board& board, int from, std::vector<Move>& moves) {
    static_cast<void>(board);
    static_cast<void>(from);
    static_cast<void>(moves);
}

void generate_knight_moves(const Board& board, int from, std::vector<Move>& moves) {
    constexpr int knight_offsets[8][2] = {
        { 1,  2},
        { 2,  1},
        { 2, -1},
        { 1, -2},
        {-1, -2},
        {-2, -1},
        {-2,  1},
        {-1,  2}
    };

    int from_file = file_of(from);
    int from_rank = rank_of(from);

    for (const auto& offset : knight_offsets) {
        int to_file = from_file + offset[0];
        int to_rank = from_rank + offset[1];

        if (to_file < 0 || to_file >= 8 || to_rank < 0 || to_rank >= 8) {
            continue;
        }

        int to = to_rank * 8 + to_file;

        if (is_occupied_by(board, to, board.side_to_move)) {
            continue;
        }

        moves.push_back(Move{from, to, MoveType::Normal, PieceType::None});
    }
}

void generate_bishop_moves(const Board& board, int from, std::vector<Move>& moves) {
    static_cast<void>(board);
    static_cast<void>(from);
    static_cast<void>(moves);
}

void generate_rook_moves(const Board& board, int from, std::vector<Move>& moves) {
    static_cast<void>(board);
    static_cast<void>(from);
    static_cast<void>(moves);
}

void generate_queen_moves(const Board& board, int from, std::vector<Move>& moves) {
    static_cast<void>(board);
    static_cast<void>(from);
    static_cast<void>(moves);
}

void generate_king_moves(const Board& board, int from, std::vector<Move>& moves) {
    static_cast<void>(board);
    static_cast<void>(from);
    static_cast<void>(moves);
}

} 

}
