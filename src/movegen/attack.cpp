#include "chess/movegen/attack.hpp"

namespace chess {

namespace {

bool is_attacked_by_pawn(const Board& board, int square, Color by_color) {
    int target_file = file_of(square);
    int target_rank = rank_of(square);
    int pawn_rank = target_rank + (by_color == Color::White ? -1 : 1);

    for (int file_delta : {-1, 1}) {
        int pawn_file = target_file + file_delta;

        if (!is_valid_file_rank(pawn_file, pawn_rank)) {
            continue;
        }

        int pawn_square = pawn_rank * 8 + pawn_file;
        Piece piece = piece_at(board, pawn_square);

        if (piece.type == PieceType::Pawn && piece.color == by_color) {
            return true;
        }
    }

    return false;
}

bool is_attacked_by_knight(const Board& board, int square, Color by_color) {
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

    int target_file = file_of(square);
    int target_rank = rank_of(square);

    for (const auto& offset : knight_offsets) {
        int from_file = target_file + offset[0];
        int from_rank = target_rank + offset[1];

        if (!is_valid_file_rank(from_file, from_rank)) {
            continue;
        }

        int from = from_rank * 8 + from_file;
        Piece piece = piece_at(board, from);

        if (piece.type == PieceType::Knight && piece.color == by_color) {
            return true;
        }
    }

    return false;
}

bool is_attacked_by_king(const Board& board, int square, Color by_color) {
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

    int target_file = file_of(square);
    int target_rank = rank_of(square);

    for (const auto& offset : king_offsets) {
        int from_file = target_file + offset[0];
        int from_rank = target_rank + offset[1];

        if (!is_valid_file_rank(from_file, from_rank)) {
            continue;
        }

        int from = from_rank * 8 + from_file;
        Piece piece = piece_at(board, from);

        if (piece.type == PieceType::King && piece.color == by_color) {
            return true;
        }
    }

    return false;
}

bool is_attacked_by_slider(
    const Board& board,
    int square,
    Color by_color,
    const int directions[][2],
    int direction_count,
    PieceType first_attacker,
    PieceType second_attacker) {
    int target_file = file_of(square);
    int target_rank = rank_of(square);

    for (int direction_index = 0; direction_index < direction_count; ++direction_index) {
        int file_delta = directions[direction_index][0];
        int rank_delta = directions[direction_index][1];
        int from_file = target_file + file_delta;
        int from_rank = target_rank + rank_delta;

        while (is_valid_file_rank(from_file, from_rank)) {
            int from = from_rank * 8 + from_file;
            Piece piece = piece_at(board, from);

            if (is_empty(piece)) {
                from_file += file_delta;
                from_rank += rank_delta;
                continue;
            }

            return piece.color == by_color && (piece.type == first_attacker || piece.type == second_attacker);
        }
    }

    return false;
}

bool is_attacked_by_bishop_or_queen(const Board& board, int square, Color by_color) {
    constexpr int directions[4][2] = {
        { 1,  1},
        { 1, -1},
        {-1, -1},
        {-1,  1}
    };

    return is_attacked_by_slider(board, square, by_color, directions, 4, PieceType::Bishop, PieceType::Queen);
}

bool is_attacked_by_rook_or_queen(const Board& board, int square, Color by_color) {
    constexpr int directions[4][2] = {
        { 1,  0},
        { 0, -1},
        {-1,  0},
        { 0,  1}
    };

    return is_attacked_by_slider(board, square, by_color, directions, 4, PieceType::Rook, PieceType::Queen);
}

} 

bool is_square_attacked(const Board& board, int square, Color by_color) {
    if (!is_valid_square(square)) {
        return false;
    }

    return is_attacked_by_pawn(board, square, by_color)
        || is_attacked_by_knight(board, square, by_color)
        || is_attacked_by_king(board, square, by_color)
        || is_attacked_by_bishop_or_queen(board, square, by_color)
        || is_attacked_by_rook_or_queen(board, square, by_color);
}

int king_square(const Board& board, Color color) {
    for (int square = 0; square < 64; ++square) {
        Piece piece = piece_at(board, square);

        if (piece.type == PieceType::King && piece.color == color) {
            return square;
        }
    }

    return NoSquare;
}

bool is_in_check(const Board& board, Color color) {
    int king = king_square(board, color);

    if (king == NoSquare) {
        return false;
    }

    return is_square_attacked(board, king, opposite(color));
}

}
