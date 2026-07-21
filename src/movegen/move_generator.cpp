#include "chess/movegen/move_generator.hpp"

namespace chess {

namespace {

void generate_pawn_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_knight_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_bishop_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_rook_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_queen_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_king_moves(const Board& board, int from, std::vector<Move>& moves);
void generate_sliding_moves(const Board& board, int from, const int directions[][2], int direction_count, std::vector<Move>& moves);
void add_pawn_move(int from, int to, bool is_promotion, std::vector<Move>& moves);

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
    Color color = board.side_to_move;
    int direction = color == Color::White ? 1 : -1;
    int start_rank = color == Color::White ? 1 : 6;
    int promotion_rank = color == Color::White ? 7 : 0;
    int from_file = file_of(from);
    int from_rank = rank_of(from);

    int one_step_rank = from_rank + direction;
    if (is_valid_file_rank(from_file, one_step_rank)) {
        int one_step = one_step_rank * 8 + from_file;

        if (is_empty_square(board, one_step)) {
            add_pawn_move(from, one_step, one_step_rank == promotion_rank, moves);

            int two_step_rank = from_rank + 2 * direction;
            if (from_rank == start_rank && is_valid_file_rank(from_file, two_step_rank)) {
                int two_step = two_step_rank * 8 + from_file;

                if (is_empty_square(board, two_step)) {
                    moves.push_back(Move{from, two_step, MoveType::Normal, PieceType::None});
                }
            }
        }
    }

    for (int file_delta : {-1, 1}) {
        int to_file = from_file + file_delta;
        int to_rank = from_rank + direction;

        if (!is_valid_file_rank(to_file, to_rank)) {
            continue;
        }

        int to = to_rank * 8 + to_file;

        if (is_occupied_by(board, to, opposite(color))) {
            add_pawn_move(from, to, to_rank == promotion_rank, moves);
        } else if (to == board.en_passant_square) {
            moves.push_back(Move{from, to, MoveType::EnPassant, PieceType::None});
        }
    }
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

        if (!is_valid_file_rank(to_file, to_rank)) {
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
    constexpr int directions[4][2] = {
        { 1,  1},
        { 1, -1},
        {-1, -1},
        {-1,  1}
    };
    generate_sliding_moves(board, from, directions, 4, moves);
}

void generate_rook_moves(const Board& board, int from, std::vector<Move>& moves) {
    constexpr int directions[4][2] = {
        { 1,  0},
        { 0, -1},
        {-1,  0},
        { 0,  1}
    };
    generate_sliding_moves(board, from, directions, 4, moves);
}

void generate_queen_moves(const Board& board, int from, std::vector<Move>& moves) {
    constexpr int directions[8][2] = {
        { 1,  0},
        { 1,  1},
        { 0,  1},
        {-1,  1},
        {-1,  0},
        {-1, -1},
        { 0, -1},
        { 1, -1}
    };
    generate_sliding_moves(board, from, directions, 8, moves);
}

void generate_king_moves(const Board& board, int from, std::vector<Move>& moves) {
    constexpr int king_offsets[8][2] = {
        { 1,  0},
        { 1,  1},
        { 0,  1},
        {-1,  1},
        {-1,  0},
        {-1, -1},
        { 0, -1},
        { 1, -1}
    };

    int from_file = file_of(from);
    int from_rank = rank_of(from);

    for (const auto& offset : king_offsets) {
        int to_file = from_file + offset[0];
        int to_rank = from_rank + offset[1];

        if (!is_valid_file_rank(to_file, to_rank)) {
            continue;
        }

        int to = to_rank * 8 + to_file;

        if (is_occupied_by(board, to, board.side_to_move)) {
            continue;
        }

        moves.push_back(Move{from, to, MoveType::Normal, PieceType::None});
    }
}

void generate_sliding_moves(const Board& board, int from, const int directions[][2], int direction_count, std::vector<Move>& moves) {
    int from_file = file_of(from);
    int from_rank = rank_of(from);

    for (int direction_index = 0; direction_index < direction_count; ++direction_index) {
        int file_delta = directions[direction_index][0];
        int rank_delta = directions[direction_index][1];
        int to_file = from_file + file_delta;
        int to_rank = from_rank + rank_delta;

        while (is_valid_file_rank(to_file, to_rank)) {
            int to = to_rank * 8 + to_file;

            if (is_occupied_by(board, to, board.side_to_move)) {
                break;
            }

            moves.push_back(Move{from, to, MoveType::Normal, PieceType::None});

            if (!is_empty_square(board, to)) {
                break;
            }

            to_file += file_delta;
            to_rank += rank_delta;
        }
    }
}

void add_pawn_move(int from, int to, bool is_promotion, std::vector<Move>& moves) {
    if (!is_promotion) {
        moves.push_back(Move{from, to, MoveType::Normal, PieceType::None});
        return;
    }

    moves.push_back(Move{from, to, MoveType::Promotion, PieceType::Queen});
    moves.push_back(Move{from, to, MoveType::Promotion, PieceType::Rook});
    moves.push_back(Move{from, to, MoveType::Promotion, PieceType::Bishop});
    moves.push_back(Move{from, to, MoveType::Promotion, PieceType::Knight});
}

} 

}
