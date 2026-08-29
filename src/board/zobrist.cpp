#include "chess/board/zobrist.hpp"

#include "chess/core/square.hpp"

namespace chess {

namespace {

constexpr std::uint64_t ZobristSeed = 0x9e3779b97f4a7c15ULL;

std::uint64_t mix(std::uint64_t value) {
    value += ZobristSeed;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

int piece_index(Piece piece) {
    int type_index = 0;

    switch (piece.type) {
        case PieceType::Pawn:
            type_index = 0;
            break;
        case PieceType::Knight:
            type_index = 1;
            break;
        case PieceType::Bishop:
            type_index = 2;
            break;
        case PieceType::Rook:
            type_index = 3;
            break;
        case PieceType::Queen:
            type_index = 4;
            break;
        case PieceType::King:
            type_index = 5;
            break;
        case PieceType::None:
            return -1;
    }

    return type_index + (piece.color == Color::White ? 0 : 6);
}

}

std::uint64_t zobrist_piece_square_key(Piece piece, int square) {
    int index = piece_index(piece);

    if (index < 0) {
        return 0;
    }

    return mix(1 + static_cast<std::uint64_t>(index) * 64 + square);
}

std::uint64_t zobrist_side_to_move_key() {
    return mix(1000);
}

std::uint64_t zobrist_castling_key(int index) {
    return mix(1100 + index);
}

std::uint64_t zobrist_en_passant_key(int square) {
    return mix(1200 + square);
}

std::uint64_t zobrist_castling_rights_key(CastlingRights castling_rights) {
    std::uint64_t hash = 0;

    if (castling_rights.white_kingside) {
        hash ^= zobrist_castling_key(0);
    }

    if (castling_rights.white_queenside) {
        hash ^= zobrist_castling_key(1);
    }

    if (castling_rights.black_kingside) {
        hash ^= zobrist_castling_key(2);
    }

    if (castling_rights.black_queenside) {
        hash ^= zobrist_castling_key(3);
    }

    return hash;
}

std::uint64_t zobrist_hash(const Board& board) {
    std::uint64_t hash = 0;

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (!is_empty(piece)) {
            hash ^= zobrist_piece_square_key(piece, square);
        }
    }

    if (board.side_to_move == Color::Black) {
        hash ^= zobrist_side_to_move_key();
    }

    hash ^= zobrist_castling_rights_key(board.castling_rights);

    if (board.en_passant_square != NoSquare) {
        hash ^= zobrist_en_passant_key(board.en_passant_square);
    }

    return hash;
}

}
