#pragma once

#include <cstdint>

#include "chess/board/board.hpp"

namespace chess {

std::uint64_t zobrist_piece_square_key(Piece piece, int square);
std::uint64_t zobrist_side_to_move_key();
std::uint64_t zobrist_castling_key(int index);
std::uint64_t zobrist_en_passant_key(int square);
std::uint64_t zobrist_castling_rights_key(CastlingRights castling_rights);
std::uint64_t zobrist_hash(const Board& board);

}
