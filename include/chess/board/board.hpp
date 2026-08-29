#pragma once

#include <array>
#include <cstdint>

#include "chess/core/color.hpp"
#include "chess/core/move.hpp"
#include "chess/core/piece.hpp"
#include "chess/core/square.hpp"

namespace chess {

struct CastlingRights {
    bool white_kingside;
    bool white_queenside;
    bool black_kingside;
    bool black_queenside;
};

struct Board {
    std::array<Piece, 64> squares;
    Color side_to_move;
    CastlingRights castling_rights;
    int en_passant_square;
    int halfmove_clock;
    int fullmove_number;
    std::uint64_t position_hash;
};

struct UndoState {
    Piece moved_piece;
    Piece captured_piece;
    int captured_square;
    CastlingRights castling_rights;
    int en_passant_square;
    int halfmove_clock;
    int fullmove_number;
    std::uint64_t position_hash;
};

Piece piece_at(const Board& board, int square);
void set_piece(Board& board, int square, Piece piece);
bool is_empty_square(const Board& board, int square);
bool is_occupied_by(const Board& board, int square, Color color);

Board empty_board();

bool make_move(Board& board, Move move, UndoState& undo);
void undo_move(Board& board, Move move, const UndoState& undo);

}
